#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_PATH_LOW "/dev/BH1750L"
#define DEVICE_PATH_HIGH "/dev/BH1750H"

#define IOCTL_SET_POWER_ON _IOW('k', 1, int)
#define IOCTL_SET_POWER_OFF _IOW('k', 2, int)
#define IOCTL_READ_Continuously_Mode1 _IOR('k', 3, int)
#define IOCTL_READ_One_Mode1 _IOR('k', 4, int)
#define IOCTL_READ_Continuously_Mode2 _IOR('k', 5, int)
#define IOCTL_READ_One_Mode2 _IOR('k', 6, int)
#define IOCTL_RESET _IOW('k', 7, int)
#define IOCTL_CHANGE_Measurement_Time _IOW('k', 8, int)
#define IOCTL_READ_Continuously_Low _IOR('k', 9, int)
#define IOCTL_READ_One_Low _IOR('k', 10, int)

int main()
{
    int fd;
    int data;

    // Mở thiết bị
    fd = open(DEVICE_PATH_HIGH, O_RDWR);

    ioctl(fd, IOCTL_SET_POWER_ON, &data);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }
    data = 44;
    ioctl(fd, IOCTL_CHANGE_Measurement_Time, &data);
    while(1){
    ioctl(fd,IOCTL_READ_Continuously_Mode2 , &data);
    printf("%d [lux]\n", data); 
        // Ngủ 1 giây
    }
    // Đóng thiết bị
    close(fd);
    return 0;
}

