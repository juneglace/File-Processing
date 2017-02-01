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

int     write_people(int, int, T_PEOPLE*);

main(int argc, char **argv)
{
    FILE        *text_in;
    T_PEOPLE    people_buf;
    int         people_file;
	off_t		rec_addr;
	char		buffer[MAXBUFFSIZE];

    if(argc < 3) {
        fprintf(stderr, "USAGE: %s input_text_file output_record_file\n", argv[0]);
        exit(1);
    }

    if((FILE*)NULL == (text_in = fopen(argv[1], "r")))  {
        fprintf(stderr, "can't open %s\n", argv[1]);
        exit(1);
    }

    if((int)NULL >
       (people_file = open(argv[2], O_BINARY|O_TRUNC|O_RDWR|O_CREAT, 0644))) {
        fprintf(stderr, "can't open %s\n", argv[2]);
        exit(1);
    }

	/* ���� ��� ���ڵ带 ���� */
	header_record.avail_head = -1;
	header_record.count = 0;
	if(sizeof(header_record) != write(people_file, (char*)&header_record, sizeof(header_record))) {
		fprintf(stderr, "can\'t write the header record\n");
		exit(1);
	}

    while(1)    {
        if(EOF == fscanf(text_in, "%d %s %c %s %s", &(people_buf.number), 
                    people_buf.name, &(people_buf.sex), people_buf.phone,
                    people_buf.address)) break;

		/* ���� ������ ���Ͽ� �������� ���·� ���ڵ� ���� */
		if((rec_addr = write_people(people_file, num_people, &people_buf)) == eERROR) {
        	fprintf(stderr, "can\'t write the %d\'th people\n", num_people);
           	exit(1);
		}
		fprintf(stderr, "%d\'th record\'s address : %d\n", num_people+1, rec_addr);
        num_people++;
    }
    fprintf(stderr, "%d people data stored ...\n", num_people);

	/* ��� ���ڵ��� count ���� num_people�� ���� */
	header_record.count = num_people;
	header_record.avail_head = -1;

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

    fclose(text_in);
    close(people_file);
}

int	write_people(int out_file, int index, T_PEOPLE *people)
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
