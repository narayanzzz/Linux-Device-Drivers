#define RDONLY  1
#define WRONLY  2
#define RW      3 

struct pcdev_data
{
    const char* serial;
    int pem;
    int size;
};