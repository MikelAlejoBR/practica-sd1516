#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "my_dropbox_protocol.h"
#include "my_dropbox_inotify.h"

/* Este fichero ha sido tomado directamente del man de inotify. */

/* Read all available inotify events from the file descriptor 'fd'.
 * wd is the table of watch descriptors for the directories in argv.
 * argc is the length of wd and argv.
 * argv is the list of watched directories.
 * Entry 0 of wd and argv is unused. */

static int handle_events(int sock, int fd, int *wd, int wdlen, char* dir)
{
          /* Some systems cannot read integer variables if they are not
           * properly aligned. On other systems, incorrect alignment may
           * decrease performance. Hence, the buffer used for reading from
           * the inotify file descriptor should have the same alignment as
           * struct inotify_event. */
          
          char buf[EVBUFF_SIZE];
          __attribute__ ((aligned(__alignof__(struct inotify_event))));
          const struct inotify_event *event;
          int i=0;
          ssize_t len;
          char *ptr;
          
          /* Loop while events can be read from inotify file descriptor. */
          
          for (;;) {
	    
	    /* Read some events. */
	    
	    len = read(fd, buf, sizeof buf);
	    if (len == -1 && errno != EAGAIN) {
	              perror("read");
	              return(-1);
	    }
	    
	    /* If the nonblocking read() found no events to read, then
	     * it returns -1 with errno set to EAGAIN. In that case,
	     * we exit the loop. */
	    
	    if (len <= 0)
	              break;
	    
	    /* Loop over all events in the buffer */
	    
	    for (ptr = buf; ptr < buf + len;
	         ptr += sizeof(struct inotify_event) + event->len) {
	              
	              event = (const struct inotify_event *) ptr;
	              
	              /* Print event type */
	              
	              if (event->mask & IN_ATTRIB)
		        printf("Attributes changed: \n");
	              
	              if (event->mask & IN_CREATE)
	              {
		        printf("File '%s' created.\n", event->name);
		        if (event->len)
			  snprintf(buf, sizeof(buf), "%s", event->name);
		        sendFile(sock, buf);
	              }
	              
	              if (event->mask & IN_DELETE_SELF)
		        printf("File '%s' self deleted. \n", event->name);
	              
	              if (event->mask & IN_DELETE)
		        printf("File '%s' deleted. \n", event->name);
	              
	              if (event->mask & IN_MODIFY)
		        printf("File '%s' modified. \n", event->name);
	              
	              if (event->mask & IN_MOVED_FROM)
	              {
		        printf("File '%s' moved or removed. \n", event->name);
		        if (event->len)
			  snprintf(buf, sizeof(buf), "%s", event->name);
		        sendDelete(sock, buf);
	              }
	              
	              if (event->mask & IN_MOVED_TO)
	              {
		        printf(" '%s' In moved to. \n", event->name);
		        if (event->len)
			  snprintf(buf, sizeof(buf), "%s", event->name);
		        sendFile(sock, buf);
	              }
	              
	              
	              if (wd[i] == event->wd) {
		        printf("%s/", dir);
		        //break;
	              }
	              
	              /* Print type of filesystem object */
	              if (event->mask & IN_ISDIR)
		        printf(" [directory]\n\n");
	              else
		        printf(" [file]\n\n");
	         }
          }
}

int inotify(char dir[]) {
          char buf;
          int fd, i, poll_num;
          int *wd;
          nfds_t nfds;
          struct pollfd fds[2];
          i=0;
          
          /* Create the file descriptor for accessing the inotify API */
          
          fd = inotify_init1(IN_NONBLOCK);
          if (fd == -1) {
	    perror("inotify_init1");
	    return(-1);
          }
          
          /* Allocate memory for watch descriptors */
          
          wd = calloc(1, sizeof(int));
          if (wd == NULL) {
	    perror("calloc");
	    return(-1);
          }
          //printf("%d\n",i);
          wd[i] = inotify_add_watch(fd, dir, IN_ATTRIB | IN_CREATE | 
          IN_MODIFY | IN_DELETE | 
          IN_DELETE_SELF 
          | IN_MOVED_FROM | IN_MOVED_TO);
          if (wd[i] == -1) {
	    fprintf(stderr, "Cannot watch '%s'\n", dir);
	    perror("inotify_add_watch");
	    return(-1);
          }
          
          /* Prepare for polling */
          
          nfds = 2;
          
          /* Console input */
          
          fds[0].fd = STDIN_FILENO;
          fds[0].events = POLLIN;
          
          /* Inotify input */
          
          fds[1].fd = fd;
          fds[1].events = POLLIN;
          
          /* Wait for events and/or terminal input */
          
          printf("Listening for events. Use enter to exit.\n");
          while (1) {
	    poll_num = poll(fds, nfds, -1);
	    if (poll_num == -1) {
	              if (errno == EINTR)
		        continue;
	              perror("poll");
	              return(-1);
	    }
	    
	    if (poll_num > 0) {
	              
	              if (fds[0].revents & POLLIN) {
		        
		        /* Console input is available. Empty stdin and quit */
		        
		        while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
			  continue;
		        break;
	              }
	              
	              if (fds[1].revents & POLLIN) {
		        
		        /* Inotify events are available */
		        
		        /* Create a new socket to establish a connection with server */
		        int sock = getsockfd();
		        if (sock < 0)
			  return(-1);
		        
		        /* Establish connection with server (my_dropbox_server) */
		        if (sendConnect(sock))
			  return(-1);
		        
		        /* Handle events */
		        if(handle_events(sock, fd, wd, 1, dir) < 0)
			  return(-1);
		        
		        /* Close the socket (end connection with server) */
		        releasesockfd(sock);
	              }
	    }
          }
          
          /* Close inotify file descriptor */
          
          close(fd);
          free(wd);
}