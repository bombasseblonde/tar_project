//#include <filesystem> //c++17
#include <fstream>
#include <sstream>
#include <grp.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include "header.hpp"

void initiate_header(Header* header)
{
	memset((void*) header, '\0', 512);
}

void add_name(Header* header, string name)
{
	if (name == "" || name[0] == 0 || name.length()>100)
		cerr << "file name/path too long" << endl;
	else
		strcpy(header->file_name, name.c_str());
}

void add_file_mode(Header* header)
{
	sprintf(header->file_mod,"%07o",0644);
}

void add_user_id(Header* header, struct stat* buff)
{
	uid_t id = buff->st_uid;
	sprintf(header->owner_user_id, "%07o", id);
}

void add_group_id(Header* header, struct stat* buff)
{
	gid_t id = buff->st_gid;
	sprintf(header->group_user_id, "%07o", id);
}

void add_size(Header* header, struct stat* buff)
{
	off_t size = buff->st_size;
	header->size = size;
	sprintf(header->file_size,"%011o", (unsigned int)size);
}



void add_mod_time(Header* header, struct stat* buff)
{
	time_t time = buff->st_mtime;
	sprintf(header->last_mod_time,"%011lo", time);
}

void checksum(Header* header)
{
	unsigned int sum = 0;
	char* p = (char*) header;
	char* stop = p + 512;
	for (; p<header->checksum; ++p)
		sum += *p & 0xff;
	for (size_t i = 0; i<8; ++i)
	{
		sum += ' ';
		++p;
	}	
	for (; p<stop; ++p)
		sum += *p & 0xff;
	sprintf(header->checksum, "%06o", sum);
	header->checksum[6] = '\0';
	header->checksum[7] = ' ';
}

void typeflag(Header* header, struct stat* buff) //stat* buff)
{
	if(buff->st_mode & S_IFDIR)
		header->link_indicator[0]='5';
	if(buff->st_mode & S_IFREG) 
		header->link_indicator[0] = '0';
}

void ustar_indicator(Header* header)
{
	strcpy(header->ustar_indicator, "ustar ");
}

void ustar_version(Header* header)
{
	strcpy(header->ustar_version, " ");
}
void get_owner_name(Header* header)
{
	strcpy(header->owner_name, getenv("USER"));
}

void get_goup_name(Header* header, struct stat* buf)
{
	struct group *grp;
	grp = getgrgid(buf->st_gid);
	strcpy(header->group_name, grp->gr_name);
}

void make_header(char* filename, Header* header)
{
	struct stat buff;
	stat(filename, &buff);
	struct stat* cur_stat = &buff;
	initiate_header(header);	
	add_name(header, string(filename));
	add_file_mode(header);		
	add_user_id(header, cur_stat);
	add_group_id(header, cur_stat);
	add_size(header, cur_stat);
	add_mod_time(header, cur_stat);
	typeflag(header, cur_stat);
	ustar_indicator(header);
	ustar_version(header);
	get_owner_name(header);
	get_goup_name(header, cur_stat);
	checksum(header);
}

off_t add_at_end(off_t size)
{
	off_t re = 512 - (size%512);
	if (re == 512)
		re = 0;
	return re;	
}

void write_file(char* out_file, char* in_file)
{
	ifstream infile;
	infile.open(in_file);
	ofstream outfile;
	outfile.open(out_file, ios::app);
	if(infile)
	{
		Header h = Header();
		Header* header = &h;
		make_header(in_file, header);
		ofstream outfile;
	        outfile.open(out_file, ios::app);
		outfile.write((char *)header, 512);
		char c;
		while(infile.get(c))
		{
			outfile << c;
		}
		off_t complete = add_at_end(h.size);
		for (off_t i = 0; i<complete; ++i)
			outfile << '\0';
		infile.close();
		outfile.close();	
	}	
}

int main(int argc, char** argv)
{
	if(argc>2)
	{
		for(int i = 2; i< argc; ++i)
		{
			write_file(argv[1], argv[i]);
		}
		ofstream outfile;
		outfile.open(argv[1], ios::app);
		for(size_t i = 0 ; i< 1024; ++i)
		{
			outfile << '\0';
		}	
		outfile.close();
	}
}
