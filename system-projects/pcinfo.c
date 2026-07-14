#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define SYSFS_PCI_DEV "/sys/bus/pci/devices"
#define PCI_IDS_PATH "/usr/share/misc/pci.ids"
#define PATH_LEN 512

// Структура для хранения найденного имени
typedef struct {
    char vendor_name[128];
    char dev_name[128];
} pci_name_t;

/* Поиск названий в pci.ids по VID и DID
 * Возвращает 0 в случае успеха, иначе -1
 * VID и DID должны быть в hex формате */
int lookup_pci_name(unsigned int vendor_id, unsigned int dev_id, pci_name_t *res);

/* Чтения файла из sysfs. записывает ID в value
 * 0 в случае успеха, иначе -1 */
static int read_sysfs_file(const char *path, unsigned int *value);

int main(void) {

    // Открываем каталог с файлами PCI
    DIR *dir = opendir(SYSFS_PCI_DEV);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    printf("List of found PCI devices\n");
    printf("------------------------------------------\n");

    // Читаем каждую запись каталога
    while ((entry = readdir(dir))) {
        // Пропускаем себя и родительский каталог
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Формируем абсолютный путь к устройству
        char vendor_path[PATH_LEN], device_path[PATH_LEN];
        snprintf(vendor_path, PATH_LEN, "%s/%s/vendor", SYSFS_PCI_DEV, entry->d_name);
        snprintf(device_path, PATH_LEN, "%s/%s/device", SYSFS_PCI_DEV, entry->d_name);

        unsigned int vid, did;
        if (read_sysfs_file(vendor_path, &vid) < 0 || read_sysfs_file(device_path, &did) < 0)
            continue;

        // Получаем названия
        pci_name_t names;
        int found = lookup_pci_name(vid, did, &names);
        printf("Device: %s\n", entry->d_name);
        printf("\tVendor ID: 0x%04X\t", vid);
        if (found == 0 && strlen(names.vendor_name) > 0)
            printf("(%s)", names.vendor_name);
        else
            printf("(unknown vendor)");
        putchar('\n');

        printf("\tDevice ID: 0x%04X\t", did);
        if (found == 0 && strlen(names.dev_name) > 0)
            printf("(%s)", names.dev_name);
        else
            printf("(unknown device)");
        putchar('\n');

    }

    closedir(dir);
    exit(EXIT_SUCCESS);
}

int lookup_pci_name(unsigned int vendor_id, unsigned int dev_id, pci_name_t *res) {
    FILE *file;
    if (!(file = fopen(PCI_IDS_PATH, "r"))) {
        perror("fopen");
        return -1;
    }

    char line[256];
    int found_vendor = 0;
    int found_dev = 0;

    // Обнуляем имена
    res->vendor_name[0] = '\0';
    res->dev_name[0] = '\0';

    while (fgets(line, sizeof(line), file)) {
        // Убираем символ новой строки
        line[strcspn(line, "\n")] = '\0';

        // Пропускаем пустые строки и комментарии
        if (line[0] == '\0' || line[0] == '#')
            continue;

        // Определяем уровень отступа: сколько пробелов/табуляций в начале
        int indent = 0;
        while (line[indent] == ' ' || line[indent] == '\t')
            ++indent;

        // Если отступ == 0 -> vendor
        if (!indent) {
            unsigned int vid;
            char vname[128];
            if (sscanf(line, "%x %[^\n]", &vid, vname) == 2) {
                if (vid == vendor_id) {
                    strcpy(res->vendor_name, vname);
                    found_vendor = 1;
                }
            }
        }
        else if (indent == 1 && found_vendor) {
            unsigned int did;
            char dname[128];
            // Пропускаем пробелы и табуляции
            char *p = line;
            while (*p == ' ' || *p == '\t') ++p;
            if (sscanf(p, "%x %[^\n]", &did, dname) == 2) {
                if (did == dev_id) {
                    strcpy(res->dev_name, dname);
                    found_dev = 1;
                    break;
                }
            }
        }
    }
    fclose(file);
    return (found_vendor && found_dev) ? 0 : -1;
}

static int read_sysfs_file(const char *path, unsigned int *value) {
    int fd;
    // Открываем path
    if ((fd = open(path, O_RDONLY)) < 0)
        return -1;

    // Считываем значение из файла
    char buffer[32];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    if (bytes <= 0)
        return -1;
    buffer[bytes] = '\0';
    // Удаляем символ новой строки
    char *newline = strchr(buffer, '\n');
    if (newline)
        *newline = '\0';

    // Переводим символьное значение в hex формат
    char *endptr;
    *value = (unsigned int)strtol(buffer, &endptr, 16);
    if (*endptr != '\0')
        return -1;
    return 0;
}
