#include <external_firmware_update.h>
#include <zephyr/storage/flash_map.h>
#include <stdio.h>
int start_firmware_update = 1;

void firmware_update(){
    const struct flash_area* image_update_partition;
    int err = flash_area_open(PARTITION_ID(image_update_partition), &image_update_partition);

    if (err)
    {
        printf("failed to open image_update_partition\n");
        return ;
    }
    else
    {
        printf("image_update_partition is open.\n");
    }

}