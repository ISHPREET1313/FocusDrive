#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#define DEVICE "/dev/ttyACM0"
#define FILE_NAME "task.txt"
#define META_DATA "meta.txt"

struct Task{
    char Name[15];
    int duration_seconds;
};

struct Meta{
    int count;
};

int check_file(struct Meta*);
void view_tasks(struct Task*,struct Meta*);
int add_task(struct Meta*,struct Task**);
int update_task(struct Meta*,struct Task**);
int delete_task(struct Meta*,struct Task**);
void get_count(struct Meta*);
int get_tasks(struct Task**,struct Meta*);
void meta_print(struct Meta*);
void task_print(struct Task**,struct Meta*);
int sync_with_pico(struct Task *, struct Meta *);

int main(){

    printf("\n\t\t\t\x1b[1;31mWELCOME TO FOCUS DRIVE \x1b[22;39m\t\t\t\n\n");
    int exit=0;

    struct Meta Meta;
    Meta.count=0;
    if(check_file(&Meta)!=0){
        return 1;
    }
    struct Task *tasks=(struct Task *)malloc(Meta.count*sizeof(struct Task));
    if (tasks == NULL) {
        printf("Initial malloc failed");
        return 1;
    }

    int file_count=get_tasks(&tasks,&Meta);
    if(file_count!=Meta.count){
        printf("Warning: metadata corrected (%d → %d)\n",Meta.count,file_count);
        Meta.count=file_count;
        meta_print(&Meta);
    }

    sleep(1);
    while (!exit){
        int input;
        printf("\n1. View tasks\n");
        printf("2. Upadte task\n");
        printf("3. Delete task\n");
        printf("4. ADD task\n");
        printf("5. Send task to Pico\n");
        printf("6. Exit\n");
        printf("\nEnter your choice: ");
        scanf("%d",&input);
        switch (input){
        case 1:
            view_tasks(tasks,&Meta);
            break;
        case 2:
            if(update_task(&Meta,&tasks)!=0){
                printf("Not found, try again\n");
            }
            break;
        case 3:
            if(delete_task(&Meta,&tasks)!=0){
                printf("Not found, try again\n");
            }
            break;
        case 4:
            if(add_task(&Meta,&tasks)!=0){
                return 1;
            }
            break;
        case 5:
            if(sync_with_pico(tasks,&Meta)!=0)
                printf("Sync failed");
            break;
        case 6:
            exit=1;
            break;
        default:
            printf("Invalid input");
            break;
        }
    }
    free(tasks);
    return 0;
}

int check_file(struct Meta *meta_p){
    FILE*fp;
    if((fp=fopen(META_DATA,"r"))==NULL){
        printf("meta file not created so creating it\n");
        fp=fopen(META_DATA,"w");
        if(fp==NULL){
            printf("Error creating file\n");
            return 1;
        }
        else{
            meta_p->count=0;
            meta_print(meta_p);
        }
    }
    else{
        printf("Connecting to meta.txt\n\n");
        get_count(meta_p);
    }
    fclose(fp);
    fp=NULL;

    if((fp=fopen(FILE_NAME,"r"))==NULL){
        printf("task file not created so creating it\n");
        fp=fopen(FILE_NAME,"w");
        if(fp==NULL){
            printf("Error creating file\n");
            return 1;
        }
    }
    else{
        printf("Connecting to task.txt\n\n");
    }

    fclose(fp);
    fp=NULL;

    return 0;
}

void view_tasks(struct Task* task ,struct Meta* meta_p){
    printf("\x1b[1J\033[H");
    printf("This is your list of task:\n");
    for(int i=0;i<meta_p->count;i++){
        printf("Task %d\n",i+1);
        printf("\tName= %s\n",task[i].Name);
        int minutes=task[i].duration_seconds/60;
        int seconds=task[i].duration_seconds%60;
        printf("\tTime= %d:%d\n",minutes,seconds);
    }
    printf("Press ENTER to continue...");
    getchar(); 
    getchar();
    printf("\x1b[1J\033[H");
}

void get_count(struct Meta* meta_p){
    FILE *m=fopen(META_DATA,"r");
    int num;
    char text[50];
    fgets(text, sizeof(text), m);
    fgets(text, sizeof(text), m);
    sscanf(text, "\tCount=%d", &meta_p->count);
    fclose(m);
}

int add_task(struct Meta* meta_p,struct Task** task){
    printf("\x1b[1J\033[H");
    int new_count=meta_p->count+1;
    struct Task *tmp=realloc(*task,new_count*sizeof(struct Task));
    if (tmp == NULL) {
        printf("realloc failed");
        return 1;
    }
    else{
        *task=tmp;
        meta_p->count=new_count;
    }
    int min,sec;
    printf("Enter Task name: ");
    scanf("%14s",(*task)[meta_p->count-1].Name);
    printf("Enter total time Duaration in (MM:SS): ");
    scanf("%d:%d",&min,&sec);
    if(min>60 || sec>=60 || min<0 || sec<0){
        printf("Invalid time format\n");
        meta_p->count-=1;
        struct Task *tmp2=realloc(*task,meta_p->count*sizeof(struct Task));
        if (tmp2 == NULL && meta_p->count>0) {
            printf("realloc failed");
            return 1;
        }
        else{
            *task=tmp2;
        }
        return 1;
    }
    (*task)[meta_p->count-1].duration_seconds=min*60+sec;
    meta_print(meta_p);
    task_print(task,meta_p);
    printf("Task added\n");
    printf("Press ENTER to continue...");
    getchar(); 
    getchar();
    printf("\x1b[1J\033[H");
    return 0;
}

void meta_print(struct Meta* meta_p){
    FILE *m = fopen(META_DATA, "w");
    if (!m) {
        printf("Error writing meta\n");
        return;
    }
    fprintf(m,"Meta Data:\n");
    fprintf(m,"\tCount=%d",meta_p->count);
    fclose(m);
}

void task_print(struct Task** task,struct Meta *meta_p){
    FILE *t = fopen(FILE_NAME, "w");
    if (!t) {
        printf("Error writing task file\n");
        return;
    }
    for(int i=0;i<meta_p->count;i++){
        fprintf(t,"%d | %s ",i+1,(*task)[i].Name);
        int minutes=(*task)[i].duration_seconds/60;
        int seconds=(*task)[i].duration_seconds%60;
        fprintf(t,"| %d:%d\n",minutes,seconds);
    }
    fclose(t);
}

int get_tasks(struct Task** task,struct Meta* meta_p){
    FILE*t=fopen(FILE_NAME,"r");
    int index=0;
    char text[50];
    for (int i = 0; i < meta_p->count; i++) {
        int min, sec;
        if (fgets(text, sizeof(text), t) == NULL)
            break;
        if (sscanf(text, "%d | %14s | %d:%d",&index,(*task)[i].Name,&min,&sec) == 4){
            (*task)[i].duration_seconds = min * 60 + sec;
        }
    }
    fclose(t);
    return index;
}

int update_task(struct Meta*meta_p,struct Task**task){
    printf("\x1b[1J\033[H");
    char name[15];
    printf("Enter Name of Task which in Update: ");
    scanf("%14s",name);
    int index=-1;
    for(int i=0;i<meta_p->count;i++){
        if(strcmp(name,(*task)[i].Name)==0){
            index=i;
            break;
        }
    }
    if(index==-1){
        printf("Name Not found\n");
        return 1;
    }
    char input;
    printf("Want to change name(n) or duration_seconds(t):" );
    scanf(" %c",&input);
    switch (input)
    {
    case 'n':
        printf("Enter New Name: ");
        scanf("%14s",(*task)[index].Name);
        printf("Updated\n");
        break;
    case 't':
        printf("Enter new Total Time (MM:SS): ");
        int min,sec;
        scanf("%d:%d",&min,&sec);
        if(min>60 || sec>=60 || min<0 || sec<0){
            printf("Invalid time format\n");
            return 1;
        }
        (*task)[meta_p->count-1].duration_seconds=min*60+sec;
        printf("Updated\n");
        break;
    default:
        printf("Invalid Input\n");
        break;
    }
    task_print(task,meta_p);
    printf("Task Updated\n");
    printf("Press ENTER to continue...");
    getchar(); 
    getchar();
    printf("\x1b[1J\033[H");
    return 0;
}

int delete_task(struct Meta*meta_p,struct Task**task){
    printf("\x1b[1J\033[H");
    char name[15];
    printf("Enter Name of Task to delete: ");
    scanf("%14s",name);
    int index=-1;
    for(int i=0;i<meta_p->count;i++){
        if(strcmp(name,(*task)[i].Name)==0){
            index=i;
            break;
        }
    }
    if(index==-1){
        printf("Name Not found\n");
        return 1;
    }
    int new_count=meta_p->count-1;
    for(int i=index;i<new_count;i++){
        strcpy((*task)[i].Name,(*task)[i+1].Name);
        (*task)[i].duration_seconds=(*task)[i+1].duration_seconds;
    }
    if (new_count == 0) {
        free(*task);
        *task = NULL;
        meta_p->count = 0;
        meta_print(meta_p);
        task_print(task, meta_p);
        return 0;
    }   
    struct Task *tmp=realloc(*task,new_count*sizeof(struct Task));
    if (tmp == NULL) {
        printf("realloc failed\n");
        return 1;
    }
    else{
        *task=tmp;
        meta_p->count=new_count;
    }
    printf("Task deleted\n");
    meta_print(meta_p);
    task_print(task,meta_p);
    printf("Press ENTER to continue...");
    getchar(); 
    getchar();
    printf("\x1b[1J\033[H");
    return 0;
}

int sync_with_pico(struct Task* tasks,struct Meta *meta_p) {
    printf("\x1b[1J\033[H");
    int task_number;
    int fd = open(DEVICE, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        printf("Cannot open USB device");
        return 1;
    }

    
    struct termios tty;
    if(tcgetattr(fd, &tty) != 0) {
        printf("terminos error");
        close(fd);
        return 1;
    }

    cfmakeraw(&tty);
    cfsetspeed(&tty, B115200);
    tty.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &tty);

    char line[64];

    printf("This is your list of task:\n");
    for(int i=0;i<meta_p->count;i++){
        printf("Task %d\n",i+1);
        printf("\tName= %s\n",tasks[i].Name);
        int minutes=tasks[i].duration_seconds/60;
        int seconds=tasks[i].duration_seconds%60;
        printf("\tTime= %d:%d\n",minutes,seconds);
    }
    printf("\nWhich task to start: ");
    scanf("%d",&task_number);
    int min = tasks[task_number-1].duration_seconds / 60;
    int sec = tasks[task_number-1].duration_seconds % 60;
    snprintf(line, sizeof(line), "%s %d:%d\n", tasks[task_number-1].Name, min, sec);
    write(fd, line, strlen(line));
    usleep(100000);

    
    printf("Waiting for Pico response...\n");
    char buf[128];
    int n;
    while ((n = read(fd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
        if(n < sizeof(buf)-1) break;
    }

    close(fd);
    printf("\nPress ENTER to continue...");
    getchar(); 
    getchar();
    printf("\x1b[1J\033[H");
    return 0;
}
