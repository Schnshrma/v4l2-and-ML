#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>


// for initializing and shutdown functions
#include <SDL2/SDL.h>
 
// for rendering images and graphics on screen
#include <SDL2/SDL_image.h>
 
// for using SDL_Delay() functions
#include <SDL2/SDL_timer.h>




uint8_t *buffer[5];

static int xioctl(int fd, int request, void *arg){
        int r;

        do r = ioctl (fd, request, arg);
        
        while (-1 == r && EINTR == errno);
 
        return r;
}
void getCurrentImageFormat(int fd){
        
        struct v4l2_format vfor;
        vfor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
        char fourcc[5] = {0};
        int val =  ioctl(fd, VIDIOC_G_FMT, &vfor);
        if(val == 0){
                       strncpy(fourcc, (char *)&vfor.fmt.pix.pixelformat, 4);                
                       printf("Current Image Format ->  %s \n", fourcc);
        } else {
                perror("Image format error");
        }
        printf("\n--------------------------------\n");
}
void setImageFormat(int fd, int imageFormat){
        struct v4l2_format vfor;
        vfor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vfor.fmt.pix.pixelformat = imageFormat;
        char fourcc[5] = {0};
        int val =  ioctl(fd, VIDIOC_S_FMT, &vfor);
        if(val == 0){
                       strncpy(fourcc, (char *)&vfor.fmt.pix.pixelformat, 4);                
                       printf("Image Format updated to ->  %s \n", fourcc);
        } else {
                perror("Updating Image format error");
        }
        printf("\n--------------------------------\n");

}
void enumerateImageFormats(int fd){
        
        printf("\nAvailable Image formats\n");
        struct v4l2_fmtdesc fmtdesc;
        memset(&fmtdesc,0,sizeof(fmtdesc));
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};

        while (ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc) == 0)
        {    strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
             printf("  %s -> %s \n", fmtdesc.description,fourcc);
             fmtdesc.index++;
        }
        printf("\n--------------------------------\n");
}
void printUserControlDescription(int fd, int id ){
        struct v4l2_queryctrl queryctrl;

        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = V4L2_CID_BRIGHTNESS;

        if (-1 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
                if (errno != EINVAL) {
                        perror("VIDIOC_QUERYCTRL");
                        exit(EXIT_FAILURE);
                } else {
                        printf("V4L2_CID_BRIGHTNESS is not supportedn");
                }
        } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {

                printf("V4L2_CID_BRIGHTNESS is not supportedn");

        } else {
                printf("\nUser Control Information -\n");
                printf("  Id %d\n  Name %s\n  Min  %d\n  Max  %d\n  Default Value  %d\n",
                        queryctrl.id,queryctrl.name,queryctrl.minimum,queryctrl.maximum,queryctrl.default_value);
        }

        printf("\n--------------------------------\n");
}
void printUserControlCurrentValue(int fd, int id){
         struct v4l2_control control;
         control.id = id ;
        
         int val = ioctl(fd, VIDIOC_G_CTRL, &control);
         if(!val){
                printf("Current control value - %d \n" , control.value);
         } else {
                 perror("Getting value");
                 exit(EXIT_FAILURE);
         }
         printf("\n--------------------------------\n");
}
void updateUserControlValue( int fd, int id , int value){
         struct v4l2_control control;
         control.id = id;
         control.value= value;

        int val = ioctl(fd, VIDIOC_S_CTRL, &control);
        if(!val){
                printf( " value updated \n ");
        } else {
                perror("Updating value");
                exit(EXIT_FAILURE);
        }
        printf("\n--------------------------------\n");
}
void enumerate_menu(struct v4l2_queryctrl queryctrl,struct v4l2_querymenu querymenu, int fd){
    printf("  Menu items:\n");

    memset(&querymenu, 0, sizeof(querymenu));
    querymenu.id = queryctrl.id;

    for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++) {
        if (0 == ioctl(fd, VIDIOC_QUERYMENU, &querymenu)) {
            printf("  %s\n", querymenu.name);
        }
    }
}
void enumerateAllUserControls(int fd){
        struct v4l2_queryctrl queryctrl;
        struct v4l2_querymenu querymenu;
        printf("\nAll Camera Controls - \n");

        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
        while (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
                printf("Control %s\n", queryctrl.name);
                queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
        
        // memset(&queryctrl, 0, sizeof(queryctrl));

        // for (queryctrl.id = V4L2_CID_BASE; queryctrl.id < V4L2_CID_LASTP1; queryctrl.id++) {
                
        //         if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        //                 if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        //                 continue;

        //                 printf("Control -> %s\n", queryctrl.name);

        //                 if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        //                 enumerate_menu(queryctrl,querymenu,fd);
        //         } else {
        //                 if (errno == EINVAL)
        //                 continue;

        //                 perror("VIDIOC_QUERYCTRL");
        //                 exit(EXIT_FAILURE);
        //         }
        // }
        // enumerateExtendedUserControls(fd);
        printf("\n--------------------------------\n");
}
void printv4l2Input(int fd){

        struct v4l2_input ip ={};
        int val = ioctl( fd, VIDIOC_ENUMINPUT, &ip );

        printf("\nV4l2 Device Input\n");
        if(!val){
                
                printf("  Index : %d \n  Name : %s \n  Type : %d \n  Capabilities: %08x\n"
                                ,ip.index , ip.name,ip.type,ip.capabilities);
                printf("\n");
                
                int currentIndex=0; 
                ioctl(fd, VIDIOC_G_INPUT, &currentIndex);
                printf("Current Index -VIDIOC_G_INPUT - %d\n",currentIndex);
        }  else {
              perror("V4l2 Input");   
         }
        
        printf("\n--------------------------------\n");


}
void devicePriority(int fd){
        printf(" \nChanging Device Priority \n");
        enum v4l2_priority priority ; 
        ioctl(fd, VIDIOC_G_PRIORITY, &priority);
        printf("  Current device priority %d ", priority);
        printf("\n");

        const enum v4l2_priority newPriority = 3;
        ioctl(fd, VIDIOC_S_PRIORITY, &newPriority);
        
        ioctl(fd, VIDIOC_G_PRIORITY, &priority);
        printf("  Current device priority %d \n", priority);
        printf("\n");

        printf("--------------------------------\n");
}
int print_caps(int fd){
        struct v4l2_capability caps = {};
        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps))
        {
                perror("Querying Capabilities");
                return 1;
        }
 
        printf( "Driver Caps:\n"
                "  Driver: \"%s\"\n"
                "  Card: \"%s\"\n"
                "  Bus: \"%s\"\n"
                "  Version: %d.%d\n"
                "  Capabilities: %08x\n",
                caps.driver,
                caps.card,
                caps.bus_info,
                (caps.version>>16)&&0xff,
                (caps.version>>24)&&0xff,
                caps.capabilities);

        printf("\n--------------------------------\n");
        
        devicePriority(fd);
        printv4l2Input(fd);
        // printv4l2Output(fd);
        // currentVideoStandard(fd);
        // listingVideoStandards(fd);
        
        enumerateAllUserControls(fd);
        printUserControlDescription(fd,V4L2_CID_BRIGHTNESS);
        printUserControlCurrentValue(fd,V4L2_CID_BRIGHTNESS);
        updateUserControlValue(fd,V4L2_CID_BRIGHTNESS,0);
        printUserControlCurrentValue(fd,V4L2_CID_BRIGHTNESS);
        
        enumerateImageFormats(fd);
        getCurrentImageFormat(fd);
        setImageFormat(fd, V4L2_PIX_FMT_YUYV);   //V4L2_PIX_FMT_MJPEG;
        getCurrentImageFormat(fd);

        // getCropingAndScalingCapabilities(fd);

        
        char fourcc[5] = {0};
        
        struct v4l2_format fmt = {0};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = 640;
        fmt.fmt.pix.height = 480;

        // applyingCropingToImage(fd);
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
        
        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        {
            perror("Setting Pixel Format");
            return 1;
        }
 
        strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
        printf( "Selected Camera Mode:\n"
                "  Width: %d\n"
                "  Height: %d\n"
                "  PixFmt: %s\n"
                "  Field: %d\n",
                fmt.fmt.pix.width,
                fmt.fmt.pix.height,
                fourcc,
                fmt.fmt.pix.field);
        return 0;
}
 
int init_mmap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 5;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
   

    // VIDIOC_QUERYBUF - > Query the status of the buffer.

    for(int i = 0 ; i < 5 ; i++){
                struct v4l2_buffer buf = {0};
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
                if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                {
                        perror("Querying Buffer");
                        return 1;
                }
                buffer[i] = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

                 printf("Length: %d\nAddress: %p\n", buf.length, buffer[i]);
                 printf("Image Length: %d\n", buf.bytesused);
    }
    
 
    return 0;
}
 
int capture_image(int fd)
{
    struct v4l2_buffer buf = {0};
    int currBuffer = 0;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Query Buffer");
        return 1;
    }


    if(-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("Start Capture");
        return 1;
    }   
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tv = {0};
        tv.tv_sec = 2;
        // tv.tv_usec = 500000;
        const int screenWidth = 640;    
        const int screenHeight = 480;
        
        SDL_Window* window = SDL_CreateWindow("Web Cam", SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

        int count = 0;
        char fileName[] = "out0.jpeg";
        while(1){
                int r = select(fd+1, &fds, NULL, NULL, &tv);
        
                if(-1 == r)
                {
                        perror("Waiting for Frame");
                        return 1;
                }

                if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
                {
                        perror("Retrieving Frame");
                        return 1;
                
                }

                
                SDL_RWops* buffer_stream ;
                SDL_Surface* frame ;
                buffer_stream = SDL_RWFromMem(buffer[currBuffer], buf.length); 
                // Create a surface using the data coming out of the above stream.
                frame = IMG_Load_RW(buffer_stream, 0);

                // SDL_Surface* image = IMG_Load("out.jpeg");
                SDL_Renderer* render = SDL_CreateRenderer(window, -1, 0);
                SDL_Texture* texture1 = SDL_CreateTextureFromSurface(render, frame);

                SDL_RenderCopy(render, texture1, NULL, NULL);
                SDL_RenderPresent(render);

                SDL_UpdateWindowSurface(window);
                SDL_FreeSurface(frame);
                SDL_RWclose(buffer_stream);

                SDL_DestroyTexture(texture1);
                SDL_DestroyRenderer(render);
                
                
                // printf( " %c %c || "  ,fileName[3] , count+48 );
                // fileName[3] = count+48;

                int outfd = open(fileName, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC , 0644);
                write(outfd, buffer[currBuffer], buf.bytesused);
                close(outfd);

                currBuffer = (currBuffer+1) % 5;
                
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = currBuffer;
                // printf("%d %d || ", buf.index , currBuffer);
                if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                {
                        perror("Query Buffer");
                        return 1;
                }

        }
        
        

 
    

//      struct v4l2_format format = {0};
//      format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
//      char fourcc[5] = {0};
//       if (-1 == xioctl(fd, VIDIOC_G_FMT, &format))
//         {
//             perror("Setting Pixel Format");
            
//         }
        
        
        // // Initialise everything.
        // SDL_Init(SDL_INIT_VIDEO);
        // IMG_Init(IMG_INIT_JPG);
        
        // // Get the screen's surface.
        // SDL_Surface* screen = SDL_GetVideoMode(
        // format.fmt.pix.width,
        // format.fmt.pix.height,
        // 32, SDL_HWSURFACE
        // );
        
        // SDL_RWops* buffer_stream;
        // SDL_Surface* frame;
        // SDL_Rect position = {.x = 0, .y = 0};
        
        // // Create a stream based on our buffer.
        // buffer_stream = SDL_RWFromMem(buffer, buf.length);
        
        // // Create a surface using the data coming out of the above stream.
        // frame = IMG_Load_RW(buffer_stream, 0);
        
        // // Blit the surface and flip the screen.
        // SDL_BlitSurface(frame, NULL, screen, &position);
        // // SDL_Flip(screen);
        
        // // Free everything, and unload SDL & Co.
        // SDL_FreeSurface(frame);
        // SDL_RWclose(buffer_stream);
        // IMG_Quit();
        // SDL_Quit();
        
        

        // SDL_Window* window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, 
        // SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        
        // SDL_RWops* buffer_stream = SDL_RWFromMem(buffer, buf.length); 
        // // Create a surface using the data coming out of the above stream.
        // SDL_Surface* frame = IMG_Load_RW(buffer_stream, 0);

        // // SDL_Surface* image = IMG_Load("out.jpeg");
        // SDL_Renderer* render = SDL_CreateRenderer(window, -1, 0);
        // SDL_Texture* texture1 = SDL_CreateTextureFromSurface(render, frame);

        // SDL_RenderCopy(render, texture1, NULL, NULL);
        // SDL_RenderPresent(render);

        // SDL_UpdateWindowSurface(window);

        // SDL_Delay(5000);

        // frame = IMG_Load("out1.jpeg");
        // texture1 = SDL_CreateTextureFromSurface(render, frame);

        // SDL_RenderCopy(render, texture1, NULL, NULL);
        // SDL_RenderPresent(render);

        // SDL_UpdateWindowSurface(window);

        // SDL_Delay(5000);

        // SDL_DestroyTexture(texture1);
        // SDL_DestroyRenderer(render);
        // SDL_FreeSurface(frame);
        // SDL_DestroyWindow(window);

        // SDL_Quit();
    
    
//     printf ("saving image\n");
    
//     int outfd = open("out.jpeg", O_WRONLY | O_APPEND | O_CREAT | O_TRUNC , 0644);
//     write(outfd, buffer, buf.bytesused);
//     close(outfd);
 
    return 0;
}
 
int main()
{
        int fd;
 
        fd = open("/dev/video0", O_RDWR);
        if (fd == -1)
        {
                perror("Opening video device");
                return 1;
        }
        if(print_caps(fd))
            return 1;
        
        if(init_mmap(fd))
            return 1;
        
        if(capture_image(fd))
                return 1;
        close(fd);

        return 0;
}
