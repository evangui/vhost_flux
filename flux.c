/**
 * flux.cΪ����������Ŀ�У����������������������������������ͳ�Ƶ�һ��c�ű���
 * ������apache����������ÿ��������log�ļ�����ÿ�η��ʵ�in out���ݴ�С�������ۼ�ͳ�ơ�
 * Ȼ��ÿ��������ÿ��������Ϣ����xml��ʽ������װ���ŵ�ָ���ķ���Ŀ¼�ļ����Թ�.net������ʵ���
 * 
 * @author evan <evan_gui@163.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <time.h>
#include <string.h>
#include <dirent.h>

#define BUFFER_SIZE     1024
#define NAME_SIZE       1024
#define LINE_SIZE	        1024
#define MAX_USER_NUM  1000					//�����������ɿ�ͨ1000�������û�
#define HOUR_NUM       24
#define HOUR_ITEM_NUM  2

struct id_name_map
{
	int id;
	char uname[NAME_SIZE];
};

/**
 * ������־�ļ���ÿ�У���ȡ������Ϣ��flux_info
 */
int getLogInfo(char* file_name, int* flux_info)
{
	int  cur_user_num, line_num;
	char line_content[NAME_SIZE];					//��־�ļ�ÿ�е���Ϣ
	char map[MAX_USER_NUM][NAME_SIZE];		//�û������û���ŵĶ�Ӧ��
	char log_line_fields[4][NAME_SIZE];				//����־�ļ�ÿ�����ݷ�4���ֶν��д洢������ÿ���ֶη��� NAME_SIZE ���ַ��ռ�
	FILE *fp;

	fp = fopen(file_name, "r");
	//
	// whileѭ���У���ÿһ�е����ݽ��з���
	//
    line_num = 0;    
    cur_user_num = 0;								//��¼��ǰ�ѷ�����������û���
    while(!feof(fp))  
    {
        fgets(line_content, LINE_SIZE, fp); 
		//
		// (2.1) ���ݿո�����ÿ�е����ݣ����뵽log_line_fields��4������
        //
		char *p;
        p = line_content;
        int line_field_num=0, line_field_index=0;
        while(1)
        {
            if(*p==' ' || *p=='\0' || *p=='\n')
            {
                log_line_fields[line_field_num++][line_field_index] = '\0';
                line_field_index=0;				//�еĵ�ǰ�������ϣ�׼��������һ����
            }
            else
			{
                log_line_fields[line_field_num][line_field_index++] = *p;
            }
			if(!*p) 
				break;
            ++p;
        }
        
		//
		// (2.2) �жϵ�ǰ�����е��û����Ƿ������mapӰ�����
		//
        int k;
        int is_analized = 0;					//��ǰ�����������û����Ƿ���map���д��ڵı�־����ʼΪ����
        long c = atol(log_line_fields[3]);			//�����4���򣬼�ʱ���
        ptime = localtime(&c);
		//�����ǰ�������û������Ѿ����û���ű��д��ڣ��򽫱��s��Ϊ1
		for(k = 0; k <= cur_user_num; k++)
        {
            if(!strcmp(map[k], log_line_fields[0]))
            {//��k����¼�а������û���Ϣ
               is_analized = 1;
			   continue;
            }
        }
		//
        //(2.3) ������ֹ����û���Ϣ,��log_line_fields[0]������mapӳ����k����¼��
		//     �򽫵�ǰ���룬������Ϣ���ӵ����û�������ͳ����Ϣ��
        //
		if(is_analized == 1)
        {
			flux_info[k][ptime->tm_hour - 1][0] += atoi(log_line_fields[1]);
            flux_info[k][ptime->tm_hour - 1][1] += atoi(log_line_fields[2]);
        }
		//
        //(2.4) ������ڣ���������һ��flux_info����
        //
		else if(is_analized == 0)
        {     
            strcpy(map[cur_user_num], log_line_fields[0]);		//map���е�i���ֶδ�š��¡��û����û����ַ���
            //map[i] = log_line_fields[0];
			flux_info[cur_user_num][ptime->tm_hour - 1][0] += atoi(log_line_fields[1]);
			flux_info[cur_user_num][ptime->tm_hour - 1][1] += atoi(log_line_fields[2]);
            cur_user_num++;
        }
    }	//whileѭ������������־ÿһ�е����ݶ��������
	fclose(fp);
	return cur_user_num;
}

/**
 * ����XML�ļ��洢flux_info�е�������Ϣ��total_numΪ�����û�����
 */
void generateXml(flux_info, total_num)
{
	int fd, t, j;
	char xml_content[2048];
    char vname[50];
    char vdate[50];
    char hour_in[50], hour_out[50], filecon[50];
	char file_name[NAME_SIZE];
	char commandbuf[BUFFER_SIZE], buffer[BUFFER_SIZE], file_name[NAME_SIZE], xml_file[NAME_SIZE];
	time_t timep;									//ʱ���
	struct tm *ptime;

	/**
	 * ��ÿ���û���װǰһ��������xml���ݣ���ŵ�xml_content��
	 */
    for(j = 0; j < total_num; j++)
    {
        memset(xml_content,'\0',sizeof(xml_content));
        //strcpy(xml_content,"<Root>\n<WebHost>");
        //printf("map[%d]=%s\n",j,map[j]);
        sprintf(vname, "<sname>%s</sname>\n", map[j]);
        strcat(xml_content, vname);
		
		sprintf(vdate, "<sdate>%02d%02d%02d</sdate>\n",
			      (1900+ptime->tm_year), (1+ptime->tm_mon), ptime->tm_mday);
        strcat(xml_content,vdate);  
		
        for(t=0; t<24; t++)
        {           
            sprintf(hour_in, "<sc%d>%d</sc%d>\n", t, flux_info[j][t][0], t);
            strcat(xml_content, hour_in);
            sprintf(hour_out, "<cs%d>%d</cs%d>\n", t, flux_info[j][t][1], t);
            strcat(xml_content, hour_out);
        }
		for(t=0;t<24;t++)
		{
			sprintf(filecon,"<file%d>0</file%d>\n",t,t);
			strcat(xml_content,filecon);		
		}
        strcat(xml_content,"</WebHost>\n</Root>"); 

		//
        //����������û�Ŀ¼���򴴽���
		//
		sprintf(file_name, "/usr/local/apache/logs/fluxlogs/%s",map[j]);
        if( (dir = opendir(file_name)) == NULL )
        {
           //printf("not exist,creating dir %s\n",map[j]);
           sprintf(commandbuf, "mkdir /usr/local/apache/logs/fluxlogs/%s", map[j] );
           system(commandbuf);
           sprintf(commandbuf, "chown vhost.vhost /usr/local/apache/logs/fluxlogs/%s -R", map[j] );
           system(commandbuf);
        }	
		closedir(dir);
        //printf("%s\n",commandbuf);

        //���ɸ��û���xml�ļ�    
        time(&timep);
        timep = timep-24*60*60;
        ptime = localtime(&timep);
        sprintf(xml_file, "/usr/local/apache/logs/fluxlogs/%s/logio_%02d%02d%02d.log",
				map[j], (1900+ptime->tm_year), (1+ptime->tm_mon), ptime->tm_mday);
        //printf("%s\n",xml_file);
        fd = open(xml_file, O_WRONLY|O_CREAT);
        write(fd, xml_content, strlen(xml_content));
        close(fd); 

		//����Ȩ��
		sprintf(commandbuf, "chown vhost.vhost %s", xml_file);
        system(commandbuf);
		sprintf(commandbuf,"chmod 751 %s",xml_file);
        system(commandbuf); 
    }   
}

int main(int argc, char *argv[])
{
	/**
	 * 1. ������ر���
	 */
	int total;
	int flux_info[MAX_USER_NUM][HOUR_NUM][2];		//��¼MAX_USER_NUM���û���������Ϣ��ÿ���û�����24��Сʱ��ÿСʱ2�����ݵ�ͳ��
	char file_name[NAME_SIZE];
	DIR *dir;
	time_t timep;									//ʱ���
	struct tm *ptime;
	//struct stat filestat;							//��ʱû����

	//username[] = argv[1];
    /**
	 * 2. �õ�Ҫ��������־�ļ�
	 */
	memset(flux_info, 0, sizeof(flux_info));			    //�������û�����һ������Ϣ���
  	time(&timep);
    timep = timep-24*60*60;
    ptime = localtime(&timep); /*ȡ�õ���ʱ��*/
	sprintf(file_name, "/usr/local/apache/logs/logio_%02d%02d%02d.log",
		(1900 + ptime->tm_year), (1 + ptime->tm_mon), ptime->tm_mday); 
	/**
	 * 3. ������־�ļ���ÿ�У���ȡ������Ϣ��flux_info
	 */
	total = getLogInfo(file_name, &flux_info);	
	
    /**
	 * 4. �����û���Ϣ������ɣ������ļ��洢������Ϣ 
	 */
    generateXml(flux_info, total);
}
