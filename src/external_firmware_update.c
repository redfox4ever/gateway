#include <external_firmware_update.h>
#include <zephyr/storage/flash_map.h>
int start_firmware_update = 1;

void start_firmware_update(){
    const struct flash_area* impage_update_partition;
    int err = flash_area_open(PARTITION_ID(impage_update_partition), &impage_update_partition);

    if (erro)
    {
        printf("failed to open image_update_partition\n");
        return 1;
    }
    else
    {
        printf("image_update_partition is open.\n");
    }

}