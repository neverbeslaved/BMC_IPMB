    #include<stdio.h>  
    #include<stdlib.h>  
    #include<unistd.h>  
    #include<sys/types.h>  
    #include<sys/stat.h>  
    #include<fcntl.h>  
    #include<termios.h>  
    #include<errno.h> 
	
	#define   FALSE        -1  
    #define   TRUE          0 
	
	/*设置串口速率*/
	void set_speed(int fd, int speed){  
      int   i;   
      int   status;   
      struct termios   Opt;  
      tcgetattr(fd, &Opt);   
      for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {   
        if  (speed == name_arr[i]) {       
          tcflush(fd, TCIOFLUSH);       
          cfsetispeed(&Opt, speed_arr[i]);    
          cfsetospeed(&Opt, speed_arr[i]);     
          status = tcsetattr(fd, TCSANOW, &Opt);    
          if  (status != 0) {          
            perror("tcsetattr fd1");    
            return;       
          }      
          tcflush(fd,TCIOFLUSH);     
        }    
      }  
    }  
	
	
	/*设置数据格式*/
	int set_Parity(int fd,int databits,int stopbits,int parity)  
    {   
        struct termios options;   
        if  ( tcgetattr( fd,&options)  !=  0) {   
            perror("SetupSerial 1");       
            return(FALSE);    
        }  
        options.c_cflag &= ~CSIZE;   
        switch (databits)   
        {     
        case 7:       
            options.c_cflag |= CS7;   
            break;  
        case 8:       
            options.c_cflag |= CS8;  
            break;     
        default:      
            fprintf(stderr,"Unsupported data size\n"); return (FALSE);    
        }  
        switch (parity)   
        {     
            case 'n':  
            case 'N':      
                options.c_cflag &= ~PARENB;   /* Clear parity enable */  
                options.c_iflag &= ~INPCK;     /* Enable parity checking */   
                break;    
            case 'o':     
            case 'O':       
                options.c_cflag |= (PARODD | PARENB);   
                options.c_iflag |= INPCK;             /* Disnable parity checking */   
                break;    
            case 'e':    
            case 'E':     
                options.c_cflag |= PARENB;     /* Enable parity */      
                options.c_cflag &= ~PARODD;      
                options.c_iflag |= INPCK;       /* Disnable parity checking */  
                break;  
            case 'S':   
            case 's':  /*as no parity*/     
                options.c_cflag &= ~PARENB;  
                options.c_cflag &= ~CSTOPB;break;    
            default:     
                fprintf(stderr,"Unsupported parity\n");      
                return (FALSE);    
            }    
          
        switch (stopbits)  
        {     
            case 1:      
                options.c_cflag &= ~CSTOPB;    
                break;    
            case 2:      
                options.c_cflag |= CSTOPB;    
               break;  
            default:      
                 fprintf(stderr,"Unsupported stop bits\n");    
                 return (FALSE);   
        }   
          
        if (parity != 'n')     
            options.c_iflag |= INPCK;   
        tcflush(fd,TCIFLUSH);  
        options.c_cc[VTIME] = 150;   
        options.c_cc[VMIN] = 0;   
        if (tcsetattr(fd,TCSANOW,&options) != 0)     
        {   
            perror("SetupSerial 3");     
            return (FALSE);    
        }   
        return (TRUE);    
    }  
	
	
	
	