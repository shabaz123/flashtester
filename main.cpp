/************************************************************************
 * FlashTester
 * main.cpp
 * rev 1.0 - February 2023 - shabaz
 ************************************************************************/

// ************* header files ******************
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "SPIFBlockDevice.h"
#include "lfs_adapter.h"

// ***************** defines *******************
// Pico-Eurocard used GPIO22 for the LED.
#define LED_PIN 22
#define BUTTON_PIN 27
//Board LED
#define PICO_LED_ON gpio_put(LED_PIN, 1)
#define PICO_LED_OFF gpio_put(LED_PIN, 0)
// Inputs
#define BUTTON_UNPRESSED (gpio_get(BUTTON_PIN)!=0)
#define BUTTON_PRESSED (gpio_get(BUTTON_PIN)==0)
// misc
#define FOREVER 1
// SPI
#define SPI_FLASH_MOSI_PIN  19
#define SPI_FLASH_MISO_PIN  16
#define SPI_FLASH_SCK_PIN   18
#define SPI_FLASH_CS_PIN    17
#define SPI_FREQ 1000000

// ************ global variables ****************
// variables used by the filesystem
extern struct lfs_config filesys_config;   // contains the configuration of the filesystem, in lfs_adapter.cpp
lfs_t lfs;
lfs_file_t file;
SPIFBlockDevice spif(0, /* unused, change SPORT value in SPIFBlockDevice.h to 0 or 1 to select SPI0 or SPI1 */
                     SPI_FLASH_MOSI_PIN,
                     SPI_FLASH_MISO_PIN,
                     SPI_FLASH_SCK_PIN,
                     SPI_FLASH_CS_PIN,
                     SPI_FREQ);

// ************ function prototypes *************


// ********** functions *************************
void
print_title(void) {
    printf("\n");
    printf("FlashTester\n");
    printf("\n");
    printf("Built on %s %s\n", __DATE__, __TIME__);
    printf("\n");
}

// general-purpose long delay timer if required
void
sleep_sec(uint32_t s) {
    sleep_ms(s * 1000);
}

// board initialisation
void
board_init(void) {
    // LED on Pico board
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // button for input
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_set_pulls(BUTTON_PIN, true, false); // pullup enabled
}

// help menu
void
display_keypress_list(void) {
    printf("Keypress Commands List\n");
    printf("h   - display this help\n");

}

// waits a short period (up to 1 msec) for a keypress
void
check_for_keypress_input(void) {
    int c;
    c = getchar_timeout_us(1000);
    if (c == PICO_ERROR_TIMEOUT) {
        return;
    }
    switch (c) {
        case 'h':
            print_title();
            break;
        default:
            break;
    }
}

// ************ main function *******************
int main(void) {
    int i;

    // initialize stdio and wait for USB CDC connect
    stdio_init_all();
    sleep_ms(1000); // could remove this after debugging, or keep it in

    // print welcome message on the USB UART or Serial UART (selected in CMakelists.txt)
    print_title();
    display_keypress_list();
    board_init(); // GPIO initialisation
    PICO_LED_ON;

    spif.init(); // SPI Flash initialisation
    printf("spif size: %llu bytes\n",         spif.size());
    printf("spif read size: %llu byte(s) granularity\n",    spif.get_read_size());
    printf("spif program size: %llu byte(s) granularity\n", spif.get_program_size());
    printf("spif erase size: %llu bytes minimum\n",   spif.get_erase_size());

#ifdef DOSPIFTEST
    // This tests the SPI Flash without any filesystem
    printf("\nWrite and Read Test\n");
    // Write "Hello World!" to the first block
    printf("Writing a message to the first block...\n");
    char *buffer = (char*)malloc(spif.get_erase_size());
    sprintf(buffer, "Hello World!\n");
    spif.erase(0, spif.get_erase_size());
    spif.program(buffer, 0, spif.get_erase_size());

    printf("Writing a message to the second block...\n");
    char *buffer2 = (char*)malloc(spif.get_erase_size());
    sprintf(buffer2, "Hello Second World!\n");
    spif.erase(4096, spif.get_erase_size());
    spif.program(buffer2, 4096, spif.get_erase_size());
    for (i-0; i<spif.get_erase_size(); i++) {
        buffer[i]=0;
    }
    // Read back what was stored
    printf("Reading back what was stored...\n");
    spif.read(buffer, 0, spif.get_erase_size());
    printf("%s", buffer);
    spif.read(buffer, 4096, spif.get_erase_size());
    printf("%s", buffer);
    printf("Write and Read Test Complete!\n");
#endif // DOSPIFTEST

    // comment this out normally!
    //lfs_format(&lfs, &filesys_config);

    // File system mount
    printf("mounting..\n");
    int err = lfs_mount(&lfs, &filesys_config);
    if (err) {
        printf("reformatting..\n");
        lfs_format(&lfs, &filesys_config);
        lfs_mount(&lfs, &filesys_config);
        printf("..done\n");
    }

    // read current count
    printf("read current count..\n");
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    printf("update boot count..\n");
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    printf("close file..\n");
    lfs_file_close(&lfs, &file);

    //Read a config file "config1.txt". If it doesn't exist, create it!
    struct lfs_info info;
    lfs_ssize_t numbytes;
    uint8_t buffer[256];
    int res = lfs_stat(&lfs, "config1.txt", &info);
    if (res<0) {
        printf("config1.txt file not found, creating..\n");
        lfs_file_open(&lfs, &file, "config1.txt", LFS_O_RDWR | LFS_O_CREAT);
        lfs_file_write(&lfs, &file, "IPADDR=192.168.0.10\n", 20);
        lfs_file_close(&lfs, &file);
    } else {
        printf("config1.txt file found, reading..\n");
        lfs_file_open(&lfs, &file, "config1.txt", LFS_O_RDWR);
        numbytes = lfs_file_read(&lfs, &file, &buffer, 255);
        printf("%s", buffer);
        lfs_file_close(&lfs, &file);
    }

    // File system unmount
    printf("unmount lfs..\n");
    lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %d\n", boot_count);


    // Close the SPI Flash usage
    spif.deinit();



    while (FOREVER) {
        check_for_keypress_input();

        PICO_LED_OFF;
        sleep_ms(20);
        PICO_LED_ON;
        sleep_ms(20);
    }
}


