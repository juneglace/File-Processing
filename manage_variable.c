#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <unistd.h>
#include    <fcntl.h>

#define eNOERROR    0
#define eERROR      -1
#define eOUTOFRANGE -2

#define	MAXBUFFSIZE	512
#define MAXNAMELEN  80
#define MAXSTRLEN   200
#define PHONESIZE   20

/* 구분자 */
#define	DELIM "|"

#define PRINT_PEOPLE(p, i) \
    {fprintf(stderr, "[%d] \tname[%s], number[%d]\n", i, (p).name, (p).number); \
    fprintf(stderr, "\taddress[%s], phoneNo[%s], sex[%s]\n\n", (p).address, \
        (p).phone, ((p).sex == 'm' ? "male" : "female"));}

typedef struct {
    int     number;
    char    name[MAXNAMELEN];
    char    sex;
	char	phone[PHONESIZE];
    char    address[MAXSTRLEN];
}   T_PEOPLE;

typedef	struct {
	int		count;	/* 레코드의 수 */
	off_t	avail_head;	/* avail list의 첫번째 레코드 주소, 없으면 -1 */
}	T_HEADER;

int     	num_people=0;
T_HEADER	header_record;

int     del_write_people(int, int,int, T_PEOPLE*);
int     read_people(int, int, T_PEOPLE*);
void	unpack_people(T_PEOPLE*, char *);
int     unpack_num(T_PEOPLE *people, char *buffer);
int     write_people(int, T_PEOPLE*);

main(int argc, char **argv)
{
    FILE        *text_in;
    T_PEOPLE    people_buf, people_temp;
    int         people_file;
    int         size, pre_size, next_size, err, i;
	off_t		rec_addr, pre_addr, next_addr;
	char		buffer[MAXBUFFSIZE];
	char        check_option(char *); //파라메타를 check하는 함수 
	int         check_name = 0; // 해당 이름사람 수 저장
	int         del_count = 0; //삭제된 수 
	int         avail_size = 0; // avail_list의 사이즈를 담을 변수
	int         temp = 0;
    char	    *result=NULL; 
    int         add_people_count = 0;
	
    if(argc < 4) {
        fprintf(stderr, "USAGE: %s read_file option option_num \n", argv[0]);
        exit(1);
    }
    if((int)NULL == (people_file = open(argv[1], O_BINARY|O_RDWR))) {
        fprintf(stderr, "can\'t open %s\n", argv[1]);
        exit(1);
    }
    
	/* 헤더 레코드를 읽어서 num_people을 설정 */
	if(sizeof(header_record) != read(people_file, (char*)&header_record, sizeof(header_record))) {
		fprintf(stderr, "can\'t read the header record\n");
		exit(1);
	}
	num_people = header_record.count;

    switch(check_option(argv[2])){
    case 'p': //p옵션
          
        if(atoi(argv[3]) < 0){
             fprintf(stderr, "\tERROR: out of range, enter the number between 0 and %d\n", num_people);
             break;
                         } 
        if(atoi(argv[3]) > 0)  {
			/* menu_value 번째 레코드 판독 */
            err = read_people(people_file, atoi(argv[3]), &people_buf);
            if(err == eERROR) {
                fprintf(stderr, "can\'t get the %d\'th people\n", i+1);
                exit(1);
            }
            if(err == eNOERROR)     PRINT_PEOPLE(people_buf, atoi(argv[3]));
        } else {
			/* 파일 포인터를 헤더 레코드를 지나서 첫번째 레코드 위치로 조정 */
			if(eERROR == lseek(people_file, sizeof(header_record), SEEK_SET)) {
				fprintf(stderr, "seek error ...\n");
				exit(1);
			}

			/* 모든 레코드를 처음부터 판독 */
            for(i=0;i < num_people;i++)   {
				/* 레코드 길이를 읽음 */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				/* size값이 음수(-)는 삭제된 데이터 */
				if(size < 0){ 
                        lseek(people_file, size * (-1) , SEEK_CUR); //지워진 부분이기때문에 넘어간다. 
                        i--;
                        continue;
                }
				/* 레코드를 읽음 */        
				if(size != read(people_file, buffer, size)) {
					fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					exit(1);
				}

				unpack_people(&people_buf, (char*)buffer);
                PRINT_PEOPLE(people_buf, i+1);
            }//for
        }//else
    break;
     case 's': // s옵션 
         for(i=0;i < num_people;i++)   {
				/* 레코드 길이를 읽음 */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				
				/* size값이 음수(-)는 삭제된 데이터 */
				if(size < 0){ 
                        lseek(people_file, size * (-1) , SEEK_CUR); //지워진 부분이기때문에 넘어간다. 
                        i--;
                        continue;
                }
				
				/* 레코드를 읽음 */
				if(size != read(people_file, buffer, size)) {
					fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					exit(1);
				}

				unpack_people(&people_buf, (char*)buffer);
				if(people_buf.number == atoi(argv[3])){//학번 check 
                    PRINT_PEOPLE(people_buf, i+1);
                    exit(0);
                }   
            }//for
            //찾지 못했을때 에러 
            fprintf(stderr, "can\'t find number %s\n", argv[3]);
            exit(1);
         break;
    case 'n': // n옵션 
         for(i=0;i < num_people;i++)   {
				/* 레코드 길이를 읽음 */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				
				/* size값이 음수(-)는 삭제된 데이터 */
				if(size < 0){ 
                        lseek(people_file, size * (-1) , SEEK_CUR); //지워진 부분이기때문에 넘어간다. 
                        i--;
                        continue;
                }
				
				/* 레코드를 읽음 */
				if(size != read(people_file, buffer, size)) {
					fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					exit(1);
				}

				unpack_people(&people_buf, (char*)buffer);
				if(!strcmp(people_buf.name, argv[3])){
                    PRINT_PEOPLE(people_buf, i+1);
                    check_name++;
                }
                  
            }//for
            if(check_name == 0){ //찾는 name이 없을 시 에러메시지    
                    fprintf(stderr, "can\'t find name %s\n", argv[3]);
                    exit(1);
                } 
            else
                printf("\nTotal %d people printed \n",check_name);
         break;
    case 'd': // d옵션 
         for(i=0;i < num_people;i++)   {
				/* 레코드 길이를 읽음 */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				/* size값이 음수(-)는 삭제된 데이터 */
				if(size < 0){ //지워진 부분이기때문에 넘어간다. 
                        if(eERROR == lseek(people_file, size * (-1) , SEEK_CUR)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                        }     
                        i--;
                        continue;
                }
				
				/* 레코드를 읽음 */
				if(size != read(people_file, buffer, size)) {
					fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					exit(1);
				}
				
				unpack_people(&people_buf, (char*)buffer);
				if(people_buf.number == atoi(argv[3])){//학번 check 
                    printf("삭제한 데이터(size: %d)\n",size); 
                    del_count++; 
                    PRINT_PEOPLE(people_buf, i+1); //삭제 데이터 출력 
                    /* 이미 읽고 지나간 거리 */
                    if(eERROR == lseek(people_file,(sizeof(size) + size)  * (-1), SEEK_CUR)) {
		                fprintf(stderr, "seek error ...\n");
		                exit(1);
                    }  
                    rec_addr = lseek(people_file,0 , SEEK_CUR); //현재 위치값 저장 
                    printf("rec_addr = %d\n", rec_addr);
                    /* 헤더 레코드의 count 값을 num_people로 수정 */
                    header_record.count = num_people - del_count;
           	        /* avail_list가 비어 있을떄 */ 
                    if( header_record.avail_head == -1){
                        header_record.avail_head = rec_addr;
                        
                        if((rec_addr = del_write_people(people_file, size, -1, &people_buf)) == eERROR) {
                                 fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                                 exit(1);
                        }
                        printf("%d\n",header_record.avail_head);
                   }
                   else{
                        /* header_record.avail_head 위치의 사이즈를 읽어 온다 */ 
                        printf("head가 -1이 아님\n"); 
                        if(eERROR == lseek(people_file, header_record.avail_head, SEEK_SET)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                        }
                        pre_addr = lseek(people_file,0 , SEEK_CUR); //현재 위치값 저장 
                        if(sizeof(pre_size) != read(people_file, &pre_size, sizeof(pre_size))) {
                                  fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
			                      return(eERROR);
                        }
                        printf("pre_size = %d\n" , pre_size);
                        
                        /* 오름 차순 정렬 */ 
                        if((pre_size * -1) < size){
                                  printf("이전 사이즈보다 크다\n");   
                                  lseek(people_file, header_record.avail_head + sizeof(pre_size) , SEEK_SET);
                                  if((pre_size * -1) != read(people_file, buffer, pre_size * -1)) {
					                      fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					                      exit(1);
	                              }
	                              next_addr = unpack_num(&people_buf, (char*)buffer);
	                              printf("next_addr = %d\n", next_addr);
	                              
	                              while(next_addr != -1){
                                          printf("3번째 avail list!\n"); 
                                          lseek(people_file, next_addr , SEEK_SET);
                                          if(sizeof(next_size) != read(people_file, &next_size, sizeof(next_size))) {
					                                      fprintf(stderr, "2can\'t read size of the %d\'th record\n", i+1);
					                                      exit(1);
                                          }
                                          printf("next_size = %d\n", next_size);
                                          if((next_size * -1) < size){
                                                       printf("next 사이즈보다 크다!\n");
                                                       pre_addr = next_addr;
                                                       pre_size = next_size;
                                                       lseek(people_file, pre_addr + sizeof(pre_size) , SEEK_SET);
                                                       if((pre_size * -1) != read(people_file, buffer, pre_size * -1)) {
					                                                fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
                                                                    exit(1);
                                                       }
	                                                   next_addr = unpack_num(&people_buf, (char*)buffer);
	                                                   printf("next_addr = %d\n", next_addr);
	                                                   
                                                       continue;
                                          }
                                          else{
                                               printf("next 사이즈보다 작다!\n");
                                               lseek(people_file, pre_addr, SEEK_SET);
                                               if((pre_addr = del_write_people(people_file, pre_size * (-1), rec_addr, &people_buf)) == eERROR) {
                                                            fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                                                            exit(1);
                                               }
                                               lseek(people_file, rec_addr, SEEK_SET);
                                               if((rec_addr = del_write_people(people_file, size, next_addr, &people_buf)) == eERROR) {
                                                            fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                                                            exit(1);
                                               }
                                               // Test
                                               lseek(people_file, header_record.avail_head + sizeof(pre_size) , SEEK_SET);
                                               if((pre_size * -1) != read(people_file, buffer, pre_size * -1)) {
					                                        fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					                                        exit(1);
                                               }
	                                           next_addr = unpack_num(&people_buf, (char*)buffer);
	                                           printf("next_addr = %d\n",next_addr);
	                                            /* 파일 포인터를 처음으로 옮김 */
                                                if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                                                  fprintf(stderr, "seek error ...\n");
		                                                  exit(1);
                                                }

	                                            /* 헤더 레코드를 파일에 저장 */
	                                            if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                                          fprintf(stderr, "can\'t write the header record\n");
                                                          exit(1);
                                                }
                                                exit(0); 
                                          }
                                  }//while
                                  if(next_addr == -1){//헤더 다음에 값이 없을 경우
                                  printf("다음 값이 없을 경우 \n"); 
                                          lseek(people_file, pre_addr , SEEK_SET);
                                          printf("pre_size = %d \n" , pre_size);
                                          if((pre_addr = del_write_people(people_file, pre_size * (-1), rec_addr, &people_buf)) == eERROR) {
                                                       fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                                                       exit(1);
                                          } 
                                          lseek(people_file, rec_addr , SEEK_SET);
                                          if((rec_addr = del_write_people(people_file, size, -1, &people_buf)) == eERROR) {
                                                       fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                                                       exit(1);
                                          }
                                          /* 파일 포인터를 처음으로 옮김 */
                                          if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                                            fprintf(stderr, "seek error ...\n");
		                                            exit(1);
                                          }
                                          /* 헤더 레코드를 파일에 저장 */
	                                      if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                                    fprintf(stderr, "can\'t write the header record\n");
                                                    exit(1);
                                          } 
                                          exit(0);
                                              
                                  }
                        }
                        
                        else{// 새로운 avail  값이 더 작을 때 헤더에 놓는다. 
                             printf("새로운 avail  값이 더 작을 때 헤더에 놓는다. \n");
                             temp = header_record.avail_head;
                             header_record.avail_head = rec_addr;
                             
                             printf("rec_addr = %d\n", rec_addr);
                             printf("size = %d\n", size);
                             lseek(people_file, rec_addr , SEEK_SET);
                             if((rec_addr = del_write_people(people_file, size, temp, &people_buf)) == eERROR) {
                                 fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                                 exit(1);
                             }
                             printf("%d\n",header_record.avail_head);
                             /* 파일 포인터를 처음으로 옮김 */
                             if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                            }

	                        /* 헤더 레코드를 파일에 저장 */
	                        if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                  fprintf(stderr, "can\'t write the header record\n");
                                  exit(1);
                            }
                             exit(0);
                        }
                   }
                   /* 파일 포인터를 처음으로 옮김 */
                        if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                        }

	                    /* 헤더 레코드를 파일에 저장 */
	                    if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                  fprintf(stderr, "can\'t write the header record\n");
                                  exit(1);
                        } 
                  }//학번 check IF   
            }//for
            if(del_count == 0){
                         fprintf(stderr, "can\'t find number %s\n", argv[3]);
                         exit(1);
            }
         break;
    case 'i': // i옵션
         if((FILE*)NULL == (text_in = fopen(argv[3], "r")))  {
             fprintf(stderr, "can't open %s\n", argv[3]);
             exit(1);
         }
         while(1)    {
                         if(EOF == fscanf(text_in, "%d %s %c %s %s", &(people_buf.number), 
                                people_buf.name, &(people_buf.sex), 
                                people_buf.phone, people_buf.address)) break;
                        
                        if(header_record.avail_head  != -1){
                             
                        }
                        else{
                             lseek(people_file, sizeof(header_record) , SEEK_SET);   
                             for(i=0;i < num_people;i++)   {     
				             /* 레코드 길이를 읽음 */
				             if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					                         fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					                         exit(1);
                              }
				
				              /* size값이 음수(-)는 삭제된 데이터 */
				              if(size < 0){ 
                                      lseek(people_file, size * (-1) , SEEK_CUR); //지워진 부분이기때문에 넘어간다. 
                                      i--;
                                      continue;
                              }
				
				              /* 레코드를 읽음 */
				              if(size != read(people_file, buffer, size)) {
					                  fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					                  exit(1);
                              }

				              unpack_people(&people_temp, (char*)buffer);
				              if(people_buf.number == people_temp.number)//학번 check 
				                      break;
                              if(i == num_people-1){
                                    lseek(people_file, 0, SEEK_END);
                                    /* 이진 데이터 파일에 가변길이 형태로 레코드 저장 */
		                            if((rec_addr = write_people(people_file, &people_buf)) == eERROR) {
        	                                     fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
           	                                     exit(1);
                                    }
                                    add_people_count++;
                               }                        
                          }//for
                        }//else   
             }//while
             header_record.count = num_people + add_people_count;
             /* 파일 포인터를 처음으로 옮김 */
	         if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		               fprintf(stderr, "seek error ...\n");
		               exit(1);
             }

	         /* 헤더 레코드를 파일에 저장 */
	         if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                       fprintf(stderr, "can\'t write the header record\n");
		               exit(1);
             }
             if(add_people_count == 0) // 추가할 내용이 없을 시 출력 
                                 printf("input file의 학번의 학생이 모두 존재합니다.");
             else
                 printf("위의 %d명의 학생이 추가되었습니다.\n", add_people_count); 
             fclose(text_in);
             break;
         break;      
    default: //매개변수가 틀릴 시 오류 처리 
         printf("Please Input the right parameter('p' 's' 'n', 'd', 'i')\n");
         exit(1);              
    }//switch
   
}

int read_people(int in_file, int index, T_PEOPLE *people)
{
	int	i, size;
	char buffer[MAXBUFFSIZE];

    if(index > num_people) {
        fprintf(stderr, 
            "\tERROR: out of range, enter the number between 0 and %d\n", num_people);
        return(eOUTOFRANGE);
    }

	/* 파일 포인터를 헤더 레코드를 지나서 첫번째 레코드 위치로 조정 */
	if(eERROR == lseek(in_file, sizeof(header_record), SEEK_SET)) {
		fprintf(stderr, "seek error ...\n");
		return(eERROR);
	}

	for(i=0; i < num_people; i++) {
		/* 레코드 길이를 읽음 */
		if(sizeof(size) != read(in_file, &size, sizeof(size))) {
			fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
			return(eERROR);
		}
		
		/* 레코드를 읽음 */
		if(size != read(in_file, buffer, size)) {
			fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
			return(eERROR);
		}

		if(i == index-1) break;
	}

	unpack_people(people, (char*)buffer);

    return(eNOERROR);
}

void unpack_people(T_PEOPLE *people, char *buffer)
{
	char	*result=NULL;

	/* 구분자(DELIM)로 구분된 첫번째 필드값인 number 복사 */
	result = strtok(buffer, DELIM);
	people->number = atoi(result);

	/* 구분자(DELIM)로 구분된 두번째 필드값인 name 복사 */
	result = strtok(NULL, DELIM);
	strcpy(people->name, result);

	/* 구분자(DELIM)로 구분된 세번째 필드값인 sex 복사 */
	result = strtok(NULL, DELIM);
	people->sex = *result;

	/* 구분자(DELIM)로 구분된 네번째 필드값인 phone 복사 */
	result = strtok(NULL, DELIM);
	strcpy(people->phone, result);

	/* 구분자(DELIM)로 구분된 다섯번째 필드값인 address 복사 */
	result = strtok(NULL, DELIM);
	strcpy(people->address, result);
}

int unpack_num(T_PEOPLE *people, char *buffer)
{
	char	*result=NULL;

	/* 구분자(DELIM)로 구분된 첫번째 필드값인 number 복사 */
	result = strtok(buffer, DELIM);
	people->number = atoi(result);

    return people->number;
}

int	del_write_people(int out_file, int size, int next_size, T_PEOPLE *people)
{
    int     i, j;
    char	buffer[MAXBUFFSIZE];
	int		rec_address;
	int     buffer_size;
	/* 현재 저장할 레코드의 바이트 주소를 확인 */
	rec_address = lseek(out_file, 0, SEEK_CUR);

    size = size * -1; //삭제된 데이터 표시 
    
    /* 레코드 길이를 저장 */
	if(sizeof(size) != write(out_file, (char*)&size, sizeof(size))) {
			return(eERROR);
	}
	
	 people->number = next_size; // 다음 avail_list의 주소 
     strcpy(people->name," "); //이름 값을 지우기 위해서 space 
     strcpy(people->phone," ");//전화번호 값을 지우기 위해서 space
     strcpy(people->address, " ");//주소 값을 지우기 위해서 space

     sprintf(buffer, "%d%s%s%s%c%s%s%s%s%s", people->number, DELIM, people->name, DELIM, people->sex,
					DELIM, people->phone, DELIM, people->address, DELIM);
					
    buffer_size = strlen(buffer);
     /* 레코드를 저장 */
	if(buffer_size != write(out_file, buffer, buffer_size)) {
			return(eERROR);
	}
	
	return(rec_address);
}
	
char check_option(char *option){ //p, s, n에 따라 1,2,3으로 옵션값을 리턴합니다. 
    if(!strcmp(option, "p"))
    return 'p';
    if(!strcmp(option, "s"))
    return 's';
    if(!strcmp(option, "n"))
    return 'n';
    if(!strcmp(option, "d"))
    return 'd';
    if(!strcmp(option, "i"))
    return 'i';
}

int	write_people(int out_file, T_PEOPLE *people)
{
	char	buffer[MAXBUFFSIZE];
	int		size;
	int		rec_address;

	/* PACKING : buffer를 필드값과 구분자의 연속으로 구성 */
	sprintf(buffer, "%d%s%s%s%c%s%s%s%s%s", people->number, DELIM, people->name, DELIM, people->sex,
					DELIM, people->phone, DELIM, people->address, DELIM);
	size = strlen(buffer);

	/* 현재 저장할 레코드의 바이트 주소를 확인 */
	rec_address = lseek(out_file, 0, SEEK_CUR);

	/* 레코드 길이를 저장 */
	if(sizeof(size) != write(out_file, (char*)&size, sizeof(size))) {
			return(eERROR);
	}

	/* 레코드를 저장 */
	if(size != write(out_file, buffer, size)) {
			return(eERROR);
	}

	/* 저장한 레코드의 바이트 주소를 반환 */
	return(rec_address);
}

