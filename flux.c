/**
 * flux.c为虚拟主机项目中，辅助进行所有虚拟主机的流量输入输出统计的一个c脚本。
 * 即分析apache虚拟主机的每个主机的log文件，将每次访问的in out数据大小，进行累加统计。
 * 然后将每个主机的每日流量信息，以xml形式进行组装，放到指定的分析目录文件，以供.net程序访问调用
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
#define MAX_USER_NUM  1000					//服务器上最多可开通1000个主机用户
#define HOUR_NUM       24
#define HOUR_ITEM_NUM  2

struct id_name_map
{
	int id;
	char uname[NAME_SIZE];
};

/**
 * 遍历日志文件的每行，提取流量信息到flux_info
 */
int getLogInfo(char* file_name, int* flux_info)
{
	int  cur_user_num, line_num;
	char line_content[NAME_SIZE];					//日志文件每行的信息
	char map[MAX_USER_NUM][NAME_SIZE];		//用户名和用户编号的对应表
	char log_line_fields[4][NAME_SIZE];				//将日志文件每行内容分4个字段进行存储，其中每个字段分配 NAME_SIZE 个字符空间
	FILE *fp;

	fp = fopen(file_name, "r");
	//
	// while循环中，对每一行的内容进行分析
	//
    line_num = 0;    
    cur_user_num = 0;								//记录当前已分析到的最大用户数
    while(!feof(fp))  
    {
        fgets(line_content, LINE_SIZE, fp); 
		//
		// (2.1) 根据空格来将每行的内容，分离到log_line_fields的4个域中
        //
		char *p;
        p = line_content;
        int line_field_num=0, line_field_index=0;
        while(1)
        {
            if(*p==' ' || *p=='\0' || *p=='\n')
            {
                log_line_fields[line_field_num++][line_field_index] = '\0';
                line_field_index=0;				//行的当前域分析完毕，准备分析下一个域
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
		// (2.2) 判断当前分析行的用户，是否存在于map影射表中
		//
        int k;
        int is_analized = 0;					//当前行所分析的用户名是否在map表中存在的标志，初始为不在
        long c = atol(log_line_fields[3]);			//处理第4个域，即时间戳
        ptime = localtime(&c);
		//如果当前分析的用户名，已经在用户编号表中存在，则将标记s设为1
		for(k = 0; k <= cur_user_num; k++)
        {
            if(!strcmp(map[k], log_line_fields[0]))
            {//第k个记录中包含该用户信息
               is_analized = 1;
			   continue;
            }
        }
		//
        //(2.3) 如果出现过该用户信息,即log_line_fields[0]存在于map映射表第k个记录中
		//     则将当前流入，流出信息增加到该用户的流量统计信息中
        //
		if(is_analized == 1)
        {
			flux_info[k][ptime->tm_hour - 1][0] += atoi(log_line_fields[1]);
            flux_info[k][ptime->tm_hour - 1][1] += atoi(log_line_fields[2]);
        }
		//
        //(2.4) 如果不在，则新增加一个flux_info来存
        //
		else if(is_analized == 0)
        {     
            strcpy(map[cur_user_num], log_line_fields[0]);		//map表中第i个字段存放‘新’用户的用户名字符串
            //map[i] = log_line_fields[0];
			flux_info[cur_user_num][ptime->tm_hour - 1][0] += atoi(log_line_fields[1]);
			flux_info[cur_user_num][ptime->tm_hour - 1][1] += atoi(log_line_fields[2]);
            cur_user_num++;
        }
    }	//while循环结束，即日志每一行的内容都分析完毕
	fclose(fp);
	return cur_user_num;
}

/**
 * 建立XML文件存储flux_info中的流量信息，total_num为流量用户个数
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
	time_t timep;									//时间撮
	struct tm *ptime;

	/**
	 * 对每个用户组装前一天流量的xml数据，存放到xml_content中
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
        //如果不存在用户目录，则创建它
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

        //生成该用户的xml文件    
        time(&timep);
        timep = timep-24*60*60;
        ptime = localtime(&timep);
        sprintf(xml_file, "/usr/local/apache/logs/fluxlogs/%s/logio_%02d%02d%02d.log",
				map[j], (1900+ptime->tm_year), (1+ptime->tm_mon), ptime->tm_mday);
        //printf("%s\n",xml_file);
        fd = open(xml_file, O_WRONLY|O_CREAT);
        write(fd, xml_content, strlen(xml_content));
        close(fd); 

		//更改权限
		sprintf(commandbuf, "chown vhost.vhost %s", xml_file);
        system(commandbuf);
		sprintf(commandbuf,"chmod 751 %s",xml_file);
        system(commandbuf); 
    }   
}

int main(int argc, char *argv[])
{
	/**
	 * 1. 定义相关变量
	 */
	int total;
	int flux_info[MAX_USER_NUM][HOUR_NUM][2];		//记录MAX_USER_NUM个用户的流量信息，每个用户包含24个小时，每小时2个数据的统计
	char file_name[NAME_SIZE];
	DIR *dir;
	time_t timep;									//时间撮
	struct tm *ptime;
	//struct stat filestat;							//暂时没用上

	//username[] = argv[1];
    /**
	 * 2. 得到要分析的日志文件
	 */
	memset(flux_info, 0, sizeof(flux_info));			    //分析新用户，上一流量信息清空
  	time(&timep);
    timep = timep-24*60*60;
    ptime = localtime(&timep); /*取得当地时间*/
	sprintf(file_name, "/usr/local/apache/logs/logio_%02d%02d%02d.log",
		(1900 + ptime->tm_year), (1 + ptime->tm_mon), ptime->tm_mday); 
	/**
	 * 3. 遍历日志文件的每行，提取流量信息到flux_info
	 */
	total = getLogInfo(file_name, &flux_info);	
	
    /**
	 * 4. 所有用户信息处理完成，建立文件存储流量信息 
	 */
    generateXml(flux_info, total);
}
