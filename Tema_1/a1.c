#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

void listRec(const char *path,int size_greater,const char *name)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    
    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\nInvalid directory path");
        return;
    }
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if(size_greater>0){
                    if(S_ISREG(statbuf.st_mode)){
                        int fd = -1;
                        fd = open(fullPath, O_RDONLY);
                        if(fd == -1) {
                            perror("Could not open file");
                            return;
                        }
                        int size = lseek(fd, 0, SEEK_END);
                        close(fd);
                        if(size>size_greater){
                            printf("%s\n", fullPath);
                        }
                    }
                } else if(name!=NULL){
                    if(strstr(entry->d_name,name)==entry->d_name){
                        printf("%s\n", fullPath);
                    }
                } else {
                    printf("%s\n", fullPath);
                }
                if(S_ISDIR(statbuf.st_mode)) {
                    listRec(fullPath,size_greater,name);
                }
            }
        }
    }
    closedir(dir);
}

int listDir(const char *path,int size_greater,const char *name){
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char filePath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\nInvalid directory path");
        return -1;
    }
    printf("SUCCESS\n");
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(filePath, 512, "%s/%s", path, entry->d_name);
            if(lstat(filePath, &statbuf) == 0) {
                if(size_greater>0){
                    if(S_ISREG(statbuf.st_mode)){
                        int fd = -1;
                        fd = open(filePath, O_RDONLY);
                        if(fd == -1) {
                            perror("Could not open file");
                            return -1;
                        }
                        int size = lseek(fd, 0, SEEK_END);
                        close(fd);
                        if(size>size_greater){
                            printf("%s\n", filePath);
                        }
                    }
                } else if(name!=NULL){
                    if(strstr(entry->d_name,name)==entry->d_name){
                        printf("%s\n", filePath);
                    }
                } else {
                    printf("%s\n", filePath);
                }
            } else {
                printf("ERROR\nInvalid directory path\n");
                return -1;
            }
        }
    }
    closedir(dir);
    return 0;
}

void parseFile(const char *path){
    int fd = -1;
    char magic[3],section_name[20];
    magic[2]=0;
    section_name[19]=0;
    int header_size=0,version=0,no_of_sections=0,section_type=0,section_size=0,section_offset=0;

    fd = open(path, O_RDONLY);
    if(fd == -1) {
        perror("Could not open file");
        return;
    }
    if(read(fd, magic, 2) != 2) {
        perror("Error reading magic");
        return;
    } 
    if(strncmp(magic,"pI",2)!=0){
        printf("ERROR\nwrong magic\n");
        return;
    }
    if(read(fd, &header_size, 2) != 2) {
        perror("Error reading header_size");
        return;
    }
    if(read(fd, &version, 1) != 1){
        perror("Error reading version");
        return;
    } else {
        if(version<65 || version>184){
            printf("ERROR\nwrong version\n");  
            return;
        }
    }
    if(read(fd, &no_of_sections, 1) != 1){
        perror("Error reading sect_nr");
        return;
    } else {
        if(no_of_sections<8 || no_of_sections>11){
            printf("ERROR\nwrong sect_nr\n");   
            return;
        }
    }
    int poz=lseek(fd,0,SEEK_CUR);
    for(int i=0;i<no_of_sections;i++){
        if(read(fd, section_name, 19) != 19){
            perror("Error reading sect_name");
            return;
        }
        if(read(fd, &section_type, 1) != 1){
            perror("Error reading sect_type");
            return;
        } else {
            if(!(section_type==13 || section_type==16 || section_type==94 || section_type==25)){
                printf("ERROR\nwrong sect_types\n");   
                return;
            }
            if(read(fd, &section_offset, 4) != 4){
                perror("Error reading sect_offset");
                return;
            }
            if(read(fd, &section_size, 4) != 4){
                perror("Error reading sect_size");
                return;
            }
        }
    }
    lseek(fd,poz,SEEK_SET);
    printf("SUCCESS\nversion=%d\nnr_sections=%d\n",version,no_of_sections);
    for(int i=0;i<no_of_sections;i++){
        if(read(fd, section_name, 19) != 19){
            perror("Error reading sect_name");
            return;
        }
        if(read(fd, &section_type, 1) != 1){
            perror("Error reading sect_type");
            return;
        }if(read(fd, &section_offset, 4) != 4){
                perror("Error reading sect_offset");
                return;
        }
        if(read(fd, &section_size, 4) != 4){
            perror("Error reading sect_size");
            return;
        }
        printf("section%d: %s %d %d\n",i+1,section_name,section_type,section_size);
    }
    close(fd);
}

void extract_section(char *path,int section,int line){
    int fd=-1;
    int no_of_sections=0, header_size=0, version=0, offset=0, section_size=0, count_new_line=1, i=1;
    char magic[3],dont_care[29],name_and_type[21],c=0;
    magic[2]=0;
    dont_care[28]=0;
    name_and_type[20]=0;

    fd=open(path,O_RDONLY);
    if(fd==-1){
        printf("ERROR\ninvalid file\n");
        return;
    }
    if(read(fd,magic,2)!=2){
        perror("Error reading magic");
        return;
    }
    if(read(fd, &header_size, 2) != 2) {
        perror("Error reading header_size");
        return;
    }
    if(read(fd, &version, 1) != 1){
        perror("Error reading version");
        return;
    }
    if(read(fd,&no_of_sections,1)!=1){
        perror("Error reading no_of_sections");
        return;
    }
    if(section>no_of_sections){
        printf("ERROR\ninvalid section\n");
        return;
    }
    while(i<section){
        if(read(fd,dont_care,28)!=28){
            perror("Error reading dont_care_section_header");
            return;
        }
        i++;
    }
    if(read(fd,name_and_type,20)!=20){
        perror("Error reading dont_care_section_header");
        return;
    }
    if(read(fd,&offset,4)!=4){
        perror("Error reading offset");
        return;
    }
    if(read(fd,&section_size,4)!=4){
        perror("Error reading section_size");
        return;
    }
    lseek(fd,offset,SEEK_SET);
    for(int i=0;i<section_size;i++){
        if(read(fd,&c,1)!=1){
            perror("Could not read c");
            return;
        }
        if(c == '\n'){
            count_new_line++;
        }
    }
    if(count_new_line<line){
        printf("ERROR\ninvalid line");
        return;
    }
    lseek(fd,offset,SEEK_SET);
    count_new_line=1;
    for(int i=0;i<section_size;i++){
        if(read(fd,&c,1)!=1){
            perror("Could not read c");
            return;
        }
        if(c == '\n'){
            count_new_line++;
        }
        if(line==1){
            printf("SUCCESS\n");
            while(c!='\n'){
                printf("%c",c);
                if(read(fd,&c,1)!=1){
                    perror("Could not read c");
                    return;
                }
            }
            printf("\n");
            break;
        }
        if(count_new_line==line){
            printf("SUCCESS\n");
            if(read(fd,&c,1)!=1){
                perror("Could not read c");
                return;
            }
            while(c!='\n'){
                printf("%c",c);
                if(read(fd,&c,1)!=1){
                    perror("Could not read c");
                    return;
                }
            }
            printf("\n");
            break;
        }
    }
    close(fd);
}

int parse_for_findall(char *path){
    int fd = -1;
    int count=0;
    char magic[3],section_name[20];
    magic[2]=0;
    section_name[19]=0;
    int header_size=0,version=0,no_of_sections=0,section_type=0,section_size=0,section_offset=0;

    fd = open(path, O_RDONLY);
    if(fd == -1) {
        perror("Could not open file");
        return 0;
    }
    if(read(fd, magic, 2) != 2) {
        perror("Error reading magic");
        return 0;
    } 
    if(strncmp(magic,"pI",2)!=0){
        return 0;
    }
    if(read(fd, &header_size, 2) != 2) {
        perror("Error reading header_size");
        return 0;
    }
    if(read(fd, &version, 1) != 1){
        perror("Error reading version");
        return 0;
    } else {
        if(version<65 || version>184){
            return 0;
        }
    }
    if(read(fd, &no_of_sections, 1) != 1){
        perror("Error reading sect_nr");
        return 0;
    } else {
        if(no_of_sections<8 || no_of_sections>11){ 
            return 0;
        }
    }
    lseek(fd,0,SEEK_CUR);
    for(int i=0;i<no_of_sections;i++){
        if(read(fd, section_name, 19) != 19){
            perror("Error reading sect_name");
            return 0;
        }
        if(read(fd, &section_type, 1) != 1){
            perror("Error reading sect_type");
            return 0;
        } else {
            if(!(section_type==13 || section_type==16 || section_type==94 || section_type==25)){ 
                return 0;
            } else if(section_type==16){
                count++;
            }
            if(read(fd, &section_offset, 4) != 4){
                perror("Error reading sect_offset");
                return 0;
            }
            if(read(fd, &section_size, 4) != 4){
                perror("Error reading sect_size");
                return 0;
            }
        }
    }
    close(fd);
    if(count>=2) {
        return 1;
    }
    return 0;
}

void find_all_sf(char *path){
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    
    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\ninvalid directory path");
        return;
    }
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if(S_ISREG(statbuf.st_mode)) {
                    int ok=parse_for_findall(fullPath);
                    if(ok){
                        printf("%s\n", fullPath);
                    }
                }
                if(S_ISDIR(statbuf.st_mode)) {
                    find_all_sf(fullPath);
                }
            }
        }
    }
    closedir(dir);
}

void function(int list,int recursive,char *path,char *size_greater_char,char *name_starts_with,int parse,int extract, char *section_char, char *line_char,int findall){
    path=path+5;
    int size_greater=0, section=0, line=0;
    char name[100];

    if(size_greater_char!=NULL){
        size_greater_char=size_greater_char+13;
        sscanf(size_greater_char,"%d",&size_greater);
    }
    if(name_starts_with!=NULL){
        name_starts_with=name_starts_with+17;
        strncpy(name,name_starts_with,100); 
    }
    if(list==1){
        if(recursive==1){
            printf("SUCCESS\n");
            listRec(path,size_greater,name);
        }
        else {
            listDir(path,size_greater,name);
        }
    } else if(parse==1){
        parseFile(path);
    } else if(extract==1){
        section_char=section_char+8;
        line_char=line_char+5;
        sscanf(section_char,"%d",&section);
        sscanf(line_char,"%d",&line);
        extract_section(path,section,line);
    } else if(findall==1){
        printf("SUCCESS\n");
        find_all_sf(path);
    }
}

int main(int argc, char **argv){
    int list=0, recursive=0, parse=0, extract=0, findall=0;
    char path[512];
    char size_greater_char[50],name_starts_with[100],section[10],line[10];
    
    if(argc >= 2){
        for(int i=1;i<argc;i++){
            if(strcmp(argv[i],"variant")==0){
                printf("76804\n");
            } else if(strcmp(argv[i],"list")==0){
                list=1;
            } else if(strcmp(argv[i],"recursive")==0){
                recursive=1;
            } else if(strstr(argv[i],"path")!=NULL){
                strncpy(path,argv[i],512);
            } else if(strstr(argv[i],"size_greater")!=NULL){
                strncpy(size_greater_char,argv[i],50);      
            } else if(strstr(argv[i],"name_starts_with")!=NULL){
                strncpy(name_starts_with,argv[i],100);      
            } else if(strcmp(argv[i],"parse")==0){
                parse=1;
            } else if(strcmp(argv[i],"extract")==0){
                extract=1;
            } else if(strstr(argv[i],"section")!=NULL){
                strncpy(section,argv[i],10);      
            } else if(strstr(argv[i],"line")!=NULL){
                strncpy(line,argv[i],10);      
            } else if(strcmp(argv[i],"findall")==0){
                findall=1;
            }
        }
        function(list,recursive,path,size_greater_char,name_starts_with,parse,extract,section,line,findall);
    }
    return 0;
}