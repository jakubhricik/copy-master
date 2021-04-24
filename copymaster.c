#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

#include "options.h"


void FatalError(char c, const char* msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);
bool isAnyOption(struct CopymasterOptions cpm_options);
int getFileSize(const char *file);


int main(int argc, char* argv[])
{
    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    //-------------------------------------------------------------------
    // Kontrola hodnot prepinacov
    //-------------------------------------------------------------------

    // Vypis hodnot prepinacov odstrante z finalnej verzie
    
    //PrintCopymasterOptions(&cpm_options);
    
    //-------------------------------------------------------------------
    // Osetrenie prepinacov pred kopirovanim
    //-------------------------------------------------------------------
    
    if (cpm_options.fast && cpm_options.slow) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.create && cpm_options.overwrite) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.create && cpm_options.append) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.append && cpm_options.overwrite) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.append && cpm_options.lseek) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.overwrite && cpm_options.lseek) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.create && cpm_options.chmod) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.delete_opt && cpm_options.truncate) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    if (cpm_options.link && cpm_options.fast){
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }if (cpm_options.link && cpm_options.slow){
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }if (cpm_options.link && cpm_options.overwrite){
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }if (cpm_options.link && cpm_options.create){
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }if (cpm_options.link && cpm_options.lseek){
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }if (cpm_options.link && cpm_options.append){
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }if (cpm_options.link && cpm_options.truncate){
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    
    // TODO Nezabudnut dalsie kontroly kombinacii prepinacov ...
    
    //-------------------------------------------------------------------
    // Kopirovanie suborov
    //-------------------------------------------------------------------
    
    // TODO Implementovat kopirovanie suborov
    
    
    /*ak su nejake prpinace*/

    //inode
    if(cpm_options.inode){
        struct stat infileStat;
        stat(cpm_options.infile, &infileStat);

        if(cpm_options.inode_number != infileStat.st_ino){
            FatalError('i', "ZLY INODE", 27);
        }
        if (!S_ISREG(infileStat.st_mode)){
            FatalError('i', "ZLY TYP VSTUPNEHO SUBORU", 27);
        }
    }

    

    if (cpm_options.link){
        int infile = open(cpm_options.infile, O_RDONLY);
        if (errno == 2){
            FatalError('K', "VSTUPNY SUBOR NEEXISTUJE", 30);
        }
        if(getFileSize(cpm_options.outfile) > 0){
            FatalError('K', "VYSTUPNY SUBOR UZ EXISTUJE", 30);
        }
        link(cpm_options.infile, cpm_options.outfile);
        close(infile);
    }

    if(cpm_options.umask){

        mode_t povolena_maska = 0;
        mode_t nepovolena_maska = 0;

        int skupina = 0;
        int pravo = 0;

        for(int i = 0; i < 10; i++) {
            if (cpm_options.umask_options[i][0] == 0) {
                break;
            }

            //kontrolujem si skupinu
            if     (cpm_options.umask_options[i][0] == 'u') skupina = 6;
            else if(cpm_options.umask_options[i][0] == 'g') skupina = 3;
            else if(cpm_options.umask_options[i][0] == 'o') skupina = 0;
            else FatalError('u', "ZLA MASKA", 32);

            //kontrolujem prava
            if     (cpm_options.umask_options[i][2] == 'r') pravo = 4;
            else if(cpm_options.umask_options[i][2] == 'w') pravo = 2;
            else if(cpm_options.umask_options[i][2] == 'x') pravo = 1;
            else FatalError('u', "ZLA MASKA", 32);

            //kontrolujem operaciu
            if( cpm_options.umask_options[i][1] == '-') {
                //to co je nepovolene posuniem na miesto podla skupiny
                nepovolena_maska |= pravo << skupina;
            }
            else if( cpm_options.umask_options[i][1] == '+'){
                // to co je povolene posuniem na miesto podla skupiny
                povolena_maska |= pravo << skupina;
            }
            else FatalError('u', "ZLA MASKA", 32);

            // printf("Povolena maska: %lld \n",povolena_maska);
            // printf("Nepovolena maska: %lld \n",nepovolena_maska);

        }
            //zistime si povodnu masku
        mode_t old_mask = umask(nepovolena_maska);
        // printf("povodna maska: %lld\n", old_mask);
            //k povodnej maske pridame povolene prava 
        old_mask = old_mask & ~povolena_maska;

        // printf("maska plus povolene prava: %lld\n", old_mask);
        // printf("maska plus nepovolene prava: %lld\n", old_mask | nepovolena_maska);
        umask(old_mask | nepovolena_maska);
    }
    

    
    int infile = open(cpm_options.infile, O_RDONLY), outfile = open(cpm_options.outfile, O_WRONLY);

    //sparse
    if(cpm_options.sparse){
        //ziskanie prav prveho suboru
        struct stat fileStat;
        stat(cpm_options.infile, &fileStat);
        
        outfile = open(cpm_options.outfile, O_WRONLY | O_TRUNC | O_CREAT, fileStat.st_mode);
        
        char buffer[1];
        read(infile, &buffer, 1);

        while(read(infile, &buffer, 1) > 0){
            if(buffer[0] != '\0'){
                write(outfile, &buffer, 1);
            }
            else lseek(outfile, 1, SEEK_CUR);
        }

        truncate(cpm_options.outfile, fileStat.st_size);
        close(infile);
        close(outfile);

        return 0;
    }

    //fast
    if(cpm_options.fast){
        //fast + create
        if(cpm_options.create){
            if (cpm_options.create_mode < 1 || cpm_options.create_mode > 777){
                FatalError('c', "ZLE PRAVA", 23);
            }
            if(getFileSize(cpm_options.outfile) > 0){
                FatalError('c', "SUBOR EXISTUJE", 23);
            }
            if (outfile == -1){
                outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT , cpm_options.create_mode);
            }
        }
        //fast+ overwrite
        else if(cpm_options.overwrite){
            outfile = open(cpm_options.outfile, O_TRUNC | O_WRONLY);
            if (errno == 2){
                FatalError('o', "SUBOR NEEXISTUJE", 24);
            }
        }
        //fast + append
        else if(cpm_options.append){
            if(!(getFileSize(cpm_options.outfile) > 0)){
                FatalError('a', "SUBOR NEEXISTUJE", 22);
            }
            outfile =  open(cpm_options.outfile, O_WRONLY | O_APPEND, 0644);
        }

        //fast + lseek
        if(cpm_options.lseek){
            if(lseek(infile, cpm_options.lseek_options.pos1, SEEK_SET) == -1 ){
                FatalError('l', "CHYBA POZICIE infile", 33);
            }

            if(cpm_options.lseek_options.x == 0){
                lseek(outfile, cpm_options.lseek_options.pos2, SEEK_SET);
            }else if(cpm_options.lseek_options.x == 1){
                lseek(outfile, cpm_options.lseek_options.pos2, SEEK_CUR);
            }else if(cpm_options.lseek_options.x == 2){
                lseek(outfile, cpm_options.lseek_options.pos2, SEEK_END);
            }else{
                FatalError('l', "CHYBA POZICIE outfile", 33);
            }
        }

        if(infile == -1 || outfile == -2){
            if(cpm_options.lseek){
                FatalError('l', "INA CHYBA", 33);
            }
            else if(cpm_options.create){
                FatalError('c', "INA CHYBA", 23);
            }
            else if(cpm_options.overwrite){
                FatalError('o', "INA CHYBA", 24);
            }
            else if(cpm_options.append){
                FatalError('a', "INA CHYBA", 22);
            }
            else FatalError('f', "INA CHYBA", -1);
        }
        int fileSize = getFileSize(cpm_options.infile);
        char buffer[fileSize];
        if(read(infile, buffer, fileSize) == -1){
            
            if(cpm_options.lseek){
                FatalError('l', "INA CHYBA", 33);
            }
            else if(cpm_options.create){
                FatalError('c', "INA CHYBA", 23);
            }
            else if(cpm_options.overwrite){
                FatalError('o', "INA CHYBA", 24);
            }
            else if(cpm_options.append){
                FatalError('a', "INA CHYBA", 22);
            }
            else FatalError('f', "INA CHYBA", -1);
        } 
            
        if(write(outfile, buffer, fileSize) == -1){
            if(cpm_options.lseek){
                FatalError('l', "INA CHYBA", 33);
            }
            else if(cpm_options.create){
                FatalError('c', "INA CHYBA", 23);
            }
            else if(cpm_options.overwrite){
                FatalError('o', "INA CHYBA", 24);
            }
            else if(cpm_options.append){
                FatalError('a', "INA CHYBA", 22);
            }
            else FatalError('f', "INA CHYBA", -1);
        }
    }

    //slow 
    else if(cpm_options.slow){
        //slow + create
        if(cpm_options.create){
            if (cpm_options.create_mode < 1 || cpm_options.create_mode > 777){
                FatalError('c', "ZLE PRAVA", 23);
            }
            if(getFileSize(cpm_options.outfile) > 0){
                FatalError('c', "SUBOR EXISTUJE", 23);
            }
            if (outfile == -1){
                outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT , cpm_options.create_mode);
            }
        }

        //slow + overwrite
        else if(cpm_options.overwrite){
            outfile = open(cpm_options.outfile, O_TRUNC | O_WRONLY);
            if (errno == 2){
                FatalError('o', "SUBOR NEEXISTUJE", 24);
            }
        }

        //slow + append
        else if(cpm_options.append){
            if(!(getFileSize(cpm_options.outfile) > 0)){
                FatalError('a', "SUBOR NEEXISTUJE", 22);
            }
            outfile =  open(cpm_options.outfile, O_WRONLY | O_APPEND, 0644);
        }

        //slow + lseek
        else if(cpm_options.lseek){
            if(lseek(infile, cpm_options.lseek_options.pos1, SEEK_SET) == -1 ){
                FatalError('l', "CHYBA POZICIE infile", 33);
            }

            if(cpm_options.lseek_options.x == 0){
                lseek(outfile, cpm_options.lseek_options.pos2, SEEK_SET);
            }else if(cpm_options.lseek_options.x == 1){
                lseek(outfile, cpm_options.lseek_options.pos2, SEEK_CUR);
            }else if(cpm_options.lseek_options.x == 2){
                lseek(outfile, cpm_options.lseek_options.pos2, SEEK_END);
            }else{
                FatalError('l', "CHYBA POZICIE outfile", 33);
            }
        }
        else outfile = open(cpm_options.outfile, O_WRONLY | O_TRUNC);

        if(infile == -1 || outfile == -2){               
            if(cpm_options.lseek){
                FatalError('l', "INA CHYBA", 33);
            }
            else if(cpm_options.create){
                FatalError('c', "INA CHYBA", 23);
            }
            else if(cpm_options.overwrite){
                FatalError('o', "INA CHYBA", 24);
            }
            else if(cpm_options.append){
                FatalError('a', "INA CHYBA", 22);
            }
            else FatalError('s', "INA CHYBA", -1);
        }
        int count;
        char buffer[1];
        while((count = read(infile, &buffer, 1))>0){
            if (count == -1 || write(outfile, &buffer, 1) == -1){
                if(cpm_options.lseek){
                    FatalError('l', "INA CHYBA", 33);
                }
                else if(cpm_options.create){
                    FatalError('c', "INA CHYBA", 23);
                }
                else if(cpm_options.overwrite){
                    FatalError('o', "INA CHYBA", 24);
                }
                else if(cpm_options.append){
                    FatalError('a', "INA CHYBA", 22);
                }
                else FatalError('s', "INA CHYBA", -1);
            }
        }
    }
    
    //only create
    else if (cpm_options.create){
        if (cpm_options.create_mode < 1 || cpm_options.create_mode > 777){
            FatalError('c', "ZLE PRAVA", 23);
        }
        if(getFileSize(cpm_options.outfile) > 0){
            FatalError('c', "SUBOR EXISTUJE", 23);
        }
        if (outfile == -1){
            outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT , cpm_options.create_mode);
        }
        if(infile == -1 || outfile == -2){
            FatalError('c', "INA CHYBA", 23);
        }
        int fileSize = getFileSize(cpm_options.infile);
        char buffer[fileSize];
        if(read(infile, buffer, fileSize) == -1){
            FatalError('c', "INA CHYBA", 23);
        } 
            
        if(write(outfile, buffer, fileSize) == -1){
            FatalError('c', "INA CHYBA", 23);
        }
    }

    //only overwrite
    else if (cpm_options.overwrite){
        outfile = open(cpm_options.outfile, O_TRUNC | O_WRONLY);
        if (errno == 2){
            FatalError('o', "SUBOR NEEXISTUJE", 24);
        }

        if(infile == -1 || outfile == -2){
            FatalError('o', "INA CHYBA", 24);
        }
        
        int count;
        char buffer[1];
        while((count = read(infile, &buffer, 1))>0){
            if (count == -1 || write(outfile, &buffer, 1) == -1){
                FatalError('o', "INA CHYBA", 24);
            }
        }
    }

    //only append
    else if (cpm_options.append){
        if(!(getFileSize(cpm_options.outfile) > 0)){
            FatalError('a', "SUBOR NEEXISTUJE", 22);
        }

        outfile =  open(cpm_options.outfile, O_WRONLY | O_APPEND, 0644);
        if(infile == -1 || outfile == -2){
            FatalError('a', "INA CHYBA", 22);
        }
        
        int count;
        char buffer[1];
        while((count = read(infile, &buffer, 1))>0){
            if (count == -1 || write(outfile, &buffer, 1) == -1){
                FatalError('a', "INA CHYBA", 22);
            }
        }
    }

    //only lseek
    else if (cpm_options.lseek){
        if(infile == -1 || outfile == -2){
            FatalError('l', "INA CHYBA", 33);
        }

        int fileSize = cpm_options.lseek_options.num;
        char buffer[fileSize];

        if(lseek(infile, cpm_options.lseek_options.pos1, SEEK_SET) == -1 ){
            FatalError('l', "CHYBA POZICIE infile", 33);
        }
        
        if(read(infile, buffer, fileSize) == -1){
            FatalError('l', "INA CHYBA", 33);
        } 

        if(cpm_options.lseek_options.x == 0){
            lseek(outfile, cpm_options.lseek_options.pos2, SEEK_SET);
        }else if(cpm_options.lseek_options.x == 1){
            lseek(outfile, cpm_options.lseek_options.pos2, SEEK_CUR);
        }else if(cpm_options.lseek_options.x == 2){
            lseek(outfile, cpm_options.lseek_options.pos2, SEEK_END);
        }else{
            FatalError('l', "CHYBA POZICIE outfile", 33);
        }
 
        if(write(outfile, buffer, fileSize) == -1){
            FatalError('l', "INA CHYBA", 33);
        }
    }
    
    else if(cpm_options.truncate){
        struct stat infileStat;
        stat(cpm_options.infile, &infileStat);
        if(!S_ISREG(infileStat.st_mode)){
            FatalError('t', "INA CHYBA", 31);
        }
        if(cpm_options.truncate_size < 0){
            FatalError('t', "ZAPORNA VELKOST", 31);
        }
        if(outfile != -1){
            int count;
            char buffer[1];
            while((count = read(infile, &buffer, 1))){
                if (count == -1 || write(outfile, &buffer, 1) == -1){
                    FatalError('t', "INA CHYBA", 31);
                }
            }
        }
        truncate(cpm_options.infile, cpm_options.truncate_size);
    }

    
    //-------------------------------------------------------------------
    // Vypis adresara
    //-------------------------------------------------------------------
    
    else if (cpm_options.directory) {

        DIR *adresar ;
        adresar = opendir(cpm_options.infile);
        struct dirent *subor = NULL;
        

        if(adresar == NULL){
            close(outfile);

            if(errno == 20)FatalError('D', "VSTUPNY SUBOR NIE JE ADRESAR", 28);
            else FatalError('D', "VSTUPNY SUBOR NEEXISTUJE", 28);
            
        }
        if(outfile == -1){
            FatalError('D', "VYSTUPNY SUBOR - CHYBA", 28);
        } 
        if(chdir(cpm_options.infile) != 0){
            FatalError('D', "INA CHYBA", 28);
        }



        
        while((subor = readdir(adresar)) != NULL){
            struct stat fileStat;
            if(stat(subor->d_name, &fileStat) == -1){
                close(outfile);
                FatalError('D', "INA CHYBA", 28);
            }
            
            char string[12] = "";
            if(S_ISDIR(fileStat.st_mode)){
                string[0] = 'd';
            }
            else if(S_ISCHR(fileStat.st_mode)){
                string[0] = 'c';
            }else if(S_ISBLK(fileStat.st_mode)){
                string[0] = 'b';
            }else if(S_ISFIFO(fileStat.st_mode)){
                string[0] = 'p';
            }else if(S_ISLNK(fileStat.st_mode)){
                string[0] = 'l';
            }else if(S_ISSOCK(fileStat.st_mode)){
                string[0] = 's';
            }else {
                string[0] = '-';
            }

            string[1] = (fileStat.st_mode & S_IRUSR) ? 'r' : '-';
            string[2] = (fileStat.st_mode & S_IWUSR) ? 'w' : '-';
            string[3] = (fileStat.st_mode & S_IXUSR) ? 'x' : '-';
            string[4] = (fileStat.st_mode & S_IRGRP) ? 'r' : '-';
            string[5] = (fileStat.st_mode & S_IWGRP) ? 'w' : '-';
            string[6] = (fileStat.st_mode & S_IXGRP) ? 'x' : '-';
            string[7] = (fileStat.st_mode & S_IROTH) ? 'r' : '-';
            string[8] = (fileStat.st_mode & S_IWOTH) ? 'w' : '-';
            string[9] = (fileStat.st_mode & S_IXOTH) ? 'x' : '-';
            string[11] = '\0';

            char time[18] = "";
            strftime(time, 11, "%e-%m-%Y", localtime(&fileStat.st_mtime));

            int result = dprintf(outfile, "%s %ld %d %d %ld %s %s\n", string, fileStat.st_nlink, fileStat.st_uid, fileStat.st_gid, fileStat.st_size, time, subor->d_name); 

            if(result < 0) {
                close(outfile);
                FatalError('D', "VSTUPNY SUBOR - CHYBA", 28);
            }
        }
        close(outfile);
    }

    /* kopirujem bez prepinacov*/
    else {
        //vytvorenie infile deskriptor
        if(!(getFileSize(cpm_options.infile) > 0)){
            FatalError('B', "SUBOR NEEXISTUJE", 21);
        }
        //ziskanie prav prveho suboru
        struct stat fileStat;
        stat(cpm_options.infile, &fileStat);
        //vytvorenie outfile deskriptor a pridelenie prav prveho suboru 
        if (outfile == -1){
            outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT , fileStat.st_mode);
        }
        else{
            outfile = open(cpm_options.outfile, O_WRONLY | O_TRUNC);
        }
        if(infile == -1 || outfile == -2){
            FatalError('B', "INA CHYBA", 21);
        }
        //ziskanie velkosti suboru
        int fileSize = getFileSize(cpm_options.infile);
        char buffer[fileSize];
        if(read(infile, buffer, fileSize) == -1) 
            FatalError('B', "INA CHYBA", 21);
        if(write(outfile, buffer, fileSize) == -1)
            FatalError('B', "INA CHYBA", 21);
    }

     //chmod
    if(cpm_options.chmod){
        if(cpm_options.chmod_mode < 1 || cpm_options.chmod_mode > 777){
            FatalError('m', "ZLE PRAVA", -1);
        }
        if(outfile == -1){
            outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT);
        }
        if(infile == -1 || outfile == -2){
            FatalError('m', "INA CHYBA", -1);
        }
        chmod(cpm_options.outfile, cpm_options.chmod_mode);
    }

    //delete
    if(cpm_options.delete_opt){
        struct stat outfileStat;
        stat(cpm_options.outfile, &outfileStat);
        if(outfile == -1){
            outfile = open(cpm_options.outfile, O_WRONLY | O_CREAT);
            int fileSize = getFileSize(cpm_options.infile);
            char buffer[fileSize];
            if(read(infile, buffer, fileSize) == -1) 
                FatalError('d', "INA CHYBA", 26);
            if(write(outfile, buffer, fileSize) == -1)
                FatalError('d', "INA CHYBA", 26);
        }

        if(S_ISREG(outfileStat.st_mode)){
            remove(cpm_options.infile);
        }else{
            FatalError('d', "SUBOR NEBOL ZMAZANY", 26);
        }
    }




    close(infile);
    close(outfile);

    return 0;
}


void FatalError(char c, const char* msg, int exit_status)
{
    fprintf(stderr, "%c:%d:", c, errno); 
    fprintf(stderr, "%s:", strerror(errno));
    fprintf(stderr, "%s", msg);
    exit(exit_status);
}

void PrintCopymasterOptions(struct CopymasterOptions* cpm_options)
{
    if (cpm_options == 0)
        return;
    
    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);
    
    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);
    
    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %zu\n", cpm_options->lseek_options.num);
    
    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);
    
    printf("umask:\t%d\n", cpm_options->umask);
    for(unsigned int i=0; i<kUMASK_OPTIONS_MAX_SZ; ++i) {
        if (cpm_options->umask_options[i][0] == 0) {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }
    
    printf("link:          %d\n", cpm_options->link);
    printf("truncate:      %d\n", cpm_options->truncate);
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}

    //-------------------------------------------------------------------
    // Pomocne funkcie
    //-------------------------------------------------------------------

int getFileSize(const char *file) {
    int fd = open(file, O_RDONLY);
    int size = lseek(fd,0L,SEEK_END);
    lseek(fd,0L,SEEK_SET);
    close(fd);
    return size;
}
