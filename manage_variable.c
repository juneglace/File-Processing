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

/* ������ */
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
	int		count;	/* ���ڵ��� �� */
	off_t	avail_head;	/* avail list�� ù��° ���ڵ� �ּ�, ������ -1 */
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
	char        check_option(char *); //�Ķ��Ÿ�� check�ϴ� �Լ� 
	int         check_name = 0; // �ش� �̸���� �� ����
	int         del_count = 0; //������ �� 
	int         avail_size = 0; // avail_list�� ����� ���� ����
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
    
	/* ��� ���ڵ带 �о num_people�� ���� */
	if(sizeof(header_record) != read(people_file, (char*)&header_record, sizeof(header_record))) {
		fprintf(stderr, "can\'t read the header record\n");
		exit(1);
	}
	num_people = header_record.count;

    switch(check_option(argv[2])){
    case 'p': //p�ɼ�
          
        if(atoi(argv[3]) < 0){
             fprintf(stderr, "\tERROR: out of range, enter the number between 0 and %d\n", num_people);
             break;
                         } 
        if(atoi(argv[3]) > 0)  {
			/* menu_value ��° ���ڵ� �ǵ� */
            err = read_people(people_file, atoi(argv[3]), &people_buf);
            if(err == eERROR) {
                fprintf(stderr, "can\'t get the %d\'th people\n", i+1);
                exit(1);
            }
            if(err == eNOERROR)     PRINT_PEOPLE(people_buf, atoi(argv[3]));
        } else {
			/* ���� �����͸� ��� ���ڵ带 ������ ù��° ���ڵ� ��ġ�� ���� */
			if(eERROR == lseek(people_file, sizeof(header_record), SEEK_SET)) {
				fprintf(stderr, "seek error ...\n");
				exit(1);
			}

			/* ��� ���ڵ带 ó������ �ǵ� */
            for(i=0;i < num_people;i++)   {
				/* ���ڵ� ���̸� ���� */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				/* size���� ����(-)�� ������ ������ */
				if(size < 0){ 
                        lseek(people_file, size * (-1) , SEEK_CUR); //������ �κ��̱⶧���� �Ѿ��. 
                        i--;
                        continue;
                }
				/* ���ڵ带 ���� */        
				if(size != read(people_file, buffer, size)) {
					fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					exit(1);
				}

				unpack_people(&people_buf, (char*)buffer);
                PRINT_PEOPLE(people_buf, i+1);
            }//for
        }//else
    break;
     case 's': // s�ɼ� 
         for(i=0;i < num_people;i++)   {
				/* ���ڵ� ���̸� ���� */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				
				/* size���� ����(-)�� ������ ������ */
				if(size < 0){ 
                        lseek(people_file, size * (-1) , SEEK_CUR); //������ �κ��̱⶧���� �Ѿ��. 
                        i--;
                        continue;
                }
				
				/* ���ڵ带 ���� */
				if(size != read(people_file, buffer, size)) {
					fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					exit(1);
				}

				unpack_people(&people_buf, (char*)buffer);
				if(people_buf.number == atoi(argv[3])){//�й� check 
                    PRINT_PEOPLE(people_buf, i+1);
                    exit(0);
                }   
            }//for
            //ã�� �������� ���� 
            fprintf(stderr, "can\'t find number %s\n", argv[3]);
            exit(1);
         break;
    case 'n': // n�ɼ� 
         for(i=0;i < num_people;i++)   {
				/* ���ڵ� ���̸� ���� */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				
				/* size���� ����(-)�� ������ ������ */
				if(size < 0){ 
                        lseek(people_file, size * (-1) , SEEK_CUR); //������ �κ��̱⶧���� �Ѿ��. 
                        i--;
                        continue;
                }
				
				/* ���ڵ带 ���� */
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
            if(check_name == 0){ //ã�� name�� ���� �� �����޽���    
                    fprintf(stderr, "can\'t find name %s\n", argv[3]);
                    exit(1);
                } 
            else
                printf("\nTotal %d people printed \n",check_name);
         break;
    case 'd': // d�ɼ� 
         for(i=0;i < num_people;i++)   {
				/* ���ڵ� ���̸� ���� */
				if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					exit(1);
				}
				/* size���� ����(-)�� ������ ������ */
				if(size < 0){ //������ �κ��̱⶧���� �Ѿ��. 
                        if(eERROR == lseek(people_file, size * (-1) , SEEK_CUR)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                        }     
                        i--;
                        continue;
                }
				
				/* ���ڵ带 ���� */
				if(size != read(people_file, buffer, size)) {
					fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					exit(1);
				}
				
				unpack_people(&people_buf, (char*)buffer);
				if(people_buf.number == atoi(argv[3])){//�й� check 
                    printf("������ ������(size: %d)\n",size); 
                    del_count++; 
                    PRINT_PEOPLE(people_buf, i+1); //���� ������ ��� 
                    /* �̹� �а� ������ �Ÿ� */
                    if(eERROR == lseek(people_file,(sizeof(size) + size)  * (-1), SEEK_CUR)) {
		                fprintf(stderr, "seek error ...\n");
		                exit(1);
                    }  
                    rec_addr = lseek(people_file,0 , SEEK_CUR); //���� ��ġ�� ���� 
                    printf("rec_addr = %d\n", rec_addr);
                    /* ��� ���ڵ��� count ���� num_people�� ���� */
                    header_record.count = num_people - del_count;
           	        /* avail_list�� ��� ������ */ 
                    if( header_record.avail_head == -1){
                        header_record.avail_head = rec_addr;
                        
                        if((rec_addr = del_write_people(people_file, size, -1, &people_buf)) == eERROR) {
                                 fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
                                 exit(1);
                        }
                        printf("%d\n",header_record.avail_head);
                   }
                   else{
                        /* header_record.avail_head ��ġ�� ����� �о� �´� */ 
                        printf("head�� -1�� �ƴ�\n"); 
                        if(eERROR == lseek(people_file, header_record.avail_head, SEEK_SET)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                        }
                        pre_addr = lseek(people_file,0 , SEEK_CUR); //���� ��ġ�� ���� 
                        if(sizeof(pre_size) != read(people_file, &pre_size, sizeof(pre_size))) {
                                  fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
			                      return(eERROR);
                        }
                        printf("pre_size = %d\n" , pre_size);
                        
                        /* ���� ���� ���� */ 
                        if((pre_size * -1) < size){
                                  printf("���� ������� ũ��\n");   
                                  lseek(people_file, header_record.avail_head + sizeof(pre_size) , SEEK_SET);
                                  if((pre_size * -1) != read(people_file, buffer, pre_size * -1)) {
					                      fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					                      exit(1);
	                              }
	                              next_addr = unpack_num(&people_buf, (char*)buffer);
	                              printf("next_addr = %d\n", next_addr);
	                              
	                              while(next_addr != -1){
                                          printf("3��° avail list!\n"); 
                                          lseek(people_file, next_addr , SEEK_SET);
                                          if(sizeof(next_size) != read(people_file, &next_size, sizeof(next_size))) {
					                                      fprintf(stderr, "2can\'t read size of the %d\'th record\n", i+1);
					                                      exit(1);
                                          }
                                          printf("next_size = %d\n", next_size);
                                          if((next_size * -1) < size){
                                                       printf("next ������� ũ��!\n");
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
                                               printf("next ������� �۴�!\n");
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
	                                            /* ���� �����͸� ó������ �ű� */
                                                if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                                                  fprintf(stderr, "seek error ...\n");
		                                                  exit(1);
                                                }

	                                            /* ��� ���ڵ带 ���Ͽ� ���� */
	                                            if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                                          fprintf(stderr, "can\'t write the header record\n");
                                                          exit(1);
                                                }
                                                exit(0); 
                                          }
                                  }//while
                                  if(next_addr == -1){//��� ������ ���� ���� ���
                                  printf("���� ���� ���� ��� \n"); 
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
                                          /* ���� �����͸� ó������ �ű� */
                                          if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                                            fprintf(stderr, "seek error ...\n");
		                                            exit(1);
                                          }
                                          /* ��� ���ڵ带 ���Ͽ� ���� */
	                                      if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                                    fprintf(stderr, "can\'t write the header record\n");
                                                    exit(1);
                                          } 
                                          exit(0);
                                              
                                  }
                        }
                        
                        else{// ���ο� avail  ���� �� ���� �� ����� ���´�. 
                             printf("���ο� avail  ���� �� ���� �� ����� ���´�. \n");
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
                             /* ���� �����͸� ó������ �ű� */
                             if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                            }

	                        /* ��� ���ڵ带 ���Ͽ� ���� */
	                        if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                  fprintf(stderr, "can\'t write the header record\n");
                                  exit(1);
                            }
                             exit(0);
                        }
                   }
                   /* ���� �����͸� ó������ �ű� */
                        if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		                          fprintf(stderr, "seek error ...\n");
		                          exit(1);
                        }

	                    /* ��� ���ڵ带 ���Ͽ� ���� */
	                    if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                                  fprintf(stderr, "can\'t write the header record\n");
                                  exit(1);
                        } 
                  }//�й� check IF   
            }//for
            if(del_count == 0){
                         fprintf(stderr, "can\'t find number %s\n", argv[3]);
                         exit(1);
            }
         break;
    case 'i': // i�ɼ�
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
				             /* ���ڵ� ���̸� ���� */
				             if(sizeof(size) != read(people_file, &size, sizeof(size))) {
					                         fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
					                         exit(1);
                              }
				
				              /* size���� ����(-)�� ������ ������ */
				              if(size < 0){ 
                                      lseek(people_file, size * (-1) , SEEK_CUR); //������ �κ��̱⶧���� �Ѿ��. 
                                      i--;
                                      continue;
                              }
				
				              /* ���ڵ带 ���� */
				              if(size != read(people_file, buffer, size)) {
					                  fprintf(stderr, "can\'t read the %d\'th record\n", i+1);
					                  exit(1);
                              }

				              unpack_people(&people_temp, (char*)buffer);
				              if(people_buf.number == people_temp.number)//�й� check 
				                      break;
                              if(i == num_people-1){
                                    lseek(people_file, 0, SEEK_END);
                                    /* ���� ������ ���Ͽ� �������� ���·� ���ڵ� ���� */
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
             /* ���� �����͸� ó������ �ű� */
	         if(eERROR == lseek(people_file, 0, SEEK_SET)) {
		               fprintf(stderr, "seek error ...\n");
		               exit(1);
             }

	         /* ��� ���ڵ带 ���Ͽ� ���� */
	         if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
                       fprintf(stderr, "can\'t write the header record\n");
		               exit(1);
             }
             if(add_people_count == 0) // �߰��� ������ ���� �� ��� 
                                 printf("input file�� �й��� �л��� ��� �����մϴ�.");
             else
                 printf("���� %d���� �л��� �߰��Ǿ����ϴ�.\n", add_people_count); 
             fclose(text_in);
             break;
         break;      
    default: //�Ű������� Ʋ�� �� ���� ó�� 
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

	/* ���� �����͸� ��� ���ڵ带 ������ ù��° ���ڵ� ��ġ�� ���� */
	if(eERROR == lseek(in_file, sizeof(header_record), SEEK_SET)) {
		fprintf(stderr, "seek error ...\n");
		return(eERROR);
	}

	for(i=0; i < num_people; i++) {
		/* ���ڵ� ���̸� ���� */
		if(sizeof(size) != read(in_file, &size, sizeof(size))) {
			fprintf(stderr, "can\'t read size of the %d\'th record\n", i+1);
			return(eERROR);
		}
		
		/* ���ڵ带 ���� */
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

	/* ������(DELIM)�� ���е� ù��° �ʵ尪�� number ���� */
	result = strtok(buffer, DELIM);
	people->number = atoi(result);

	/* ������(DELIM)�� ���е� �ι�° �ʵ尪�� name ���� */
	result = strtok(NULL, DELIM);
	strcpy(people->name, result);

	/* ������(DELIM)�� ���е� ����° �ʵ尪�� sex ���� */
	result = strtok(NULL, DELIM);
	people->sex = *result;

	/* ������(DELIM)�� ���е� �׹�° �ʵ尪�� phone ���� */
	result = strtok(NULL, DELIM);
	strcpy(people->phone, result);

	/* ������(DELIM)�� ���е� �ټ���° �ʵ尪�� address ���� */
	result = strtok(NULL, DELIM);
	strcpy(people->address, result);
}

int unpack_num(T_PEOPLE *people, char *buffer)
{
	char	*result=NULL;

	/* ������(DELIM)�� ���е� ù��° �ʵ尪�� number ���� */
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
	/* ���� ������ ���ڵ��� ����Ʈ �ּҸ� Ȯ�� */
	rec_address = lseek(out_file, 0, SEEK_CUR);

    size = size * -1; //������ ������ ǥ�� 
    
    /* ���ڵ� ���̸� ���� */
	if(sizeof(size) != write(out_file, (char*)&size, sizeof(size))) {
			return(eERROR);
	}
	
	 people->number = next_size; // ���� avail_list�� �ּ� 
     strcpy(people->name," "); //�̸� ���� ����� ���ؼ� space 
     strcpy(people->phone," ");//��ȭ��ȣ ���� ����� ���ؼ� space
     strcpy(people->address, " ");//�ּ� ���� ����� ���ؼ� space

     sprintf(buffer, "%d%s%s%s%c%s%s%s%s%s", people->number, DELIM, people->name, DELIM, people->sex,
					DELIM, people->phone, DELIM, people->address, DELIM);
					
    buffer_size = strlen(buffer);
     /* ���ڵ带 ���� */
	if(buffer_size != write(out_file, buffer, buffer_size)) {
			return(eERROR);
	}
	
	return(rec_address);
}
	
char check_option(char *option){ //p, s, n�� ���� 1,2,3���� �ɼǰ��� �����մϴ�. 
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

	/* PACKING : buffer�� �ʵ尪�� �������� �������� ���� */
	sprintf(buffer, "%d%s%s%s%c%s%s%s%s%s", people->number, DELIM, people->name, DELIM, people->sex,
					DELIM, people->phone, DELIM, people->address, DELIM);
	size = strlen(buffer);

	/* ���� ������ ���ڵ��� ����Ʈ �ּҸ� Ȯ�� */
	rec_address = lseek(out_file, 0, SEEK_CUR);

	/* ���ڵ� ���̸� ���� */
	if(sizeof(size) != write(out_file, (char*)&size, sizeof(size))) {
			return(eERROR);
	}

	/* ���ڵ带 ���� */
	if(size != write(out_file, buffer, size)) {
			return(eERROR);
	}

	/* ������ ���ڵ��� ����Ʈ �ּҸ� ��ȯ */
	return(rec_address);
}

