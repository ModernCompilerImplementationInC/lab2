#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using std::string;



yfs_client::yfs_client()
{
    ec = new extent_client();

}

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
    ec = new extent_client();
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
yfs_client::filename(inum inum)
{
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        return true;
    } 
    printf("isfile: %lld is not a file\n", inum);
    return false;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */

bool
yfs_client::isdir(inum inum)
{
    // Oops! is this still correct when you implement symlink?
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_DIR) {
        printf("isdir: %lld is a dir\n", inum);
        return true;
    } 
    printf("isdir: %lld is not a dir\n", inum);
    return false;
}



int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;

    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    return r;
}


#define EXT_RPC(xx) do { \
    if ((xx) != extent_protocol::OK) { \
        printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
        r = IOERR; \
        goto release; \
    } \
} while (0)

// Only support set size of attr
int
yfs_client::setattr(inum ino, size_t size)
{
  
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */
    std::string buf;
    extent_protocol::attr a;
    extent_protocol::status ret;
    if ((ret = ec->getattr(ino, a)) != extent_protocol::OK) {
      return ret;
    }
    ec->get(ino, buf);
    if (a.size < size) 
    {
      buf += std::string(size - a.size, '\0');
    } 
    else if (a.size > size) 
    {
      buf = buf.substr(0, size);
    }
    ec->put(ino, buf);
    return r;
}

int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    bool found ;
    lookup(parent, name, found, ino_out);
    if(!found)
    {
        ec->create(extent_protocol::T_FILE, ino_out);
        string buf;
        ec->get(parent,buf);
        if (buf.size() == 0)
            buf.append(string(name) + ',' + filename(ino_out));
        else
            buf.append(',' + string(name) + ',' + filename(ino_out));
        ec->put(parent, buf);
    }
    else
        r = EXIST; 
    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    bool found;
    lookup(parent, name, found, ino_out);
    if(!found)
    {
        ec->create(extent_protocol::T_DIR, ino_out);
        string buf;
        ec->get(parent,buf);
        if(buf.size() == 0)
            buf.append(string(name) + ',' + filename(ino_out));
        else
            buf.append(',' + string(name) + ',' + filename(ino_out));
        ec->put(parent, buf);
    }
    else
        r = EXIST; 
    return r;
}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
   
	std::list<dirent> dirList;
	r = readdir(parent, dirList);
	std::list<dirent>::iterator iter;
	for(iter = dirList.begin(); iter != dirList.end(); ++iter){
		if(iter->name == std::string(name)){
			found = true;
			ino_out = iter->inum;
			return r;
		}
	}
  found = false;
	return r;
}

int
yfs_client::readdir(inum dir, std::list<dirent> &list)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
     

    string buf;
    ec->get(dir, buf);
	string filename="";
	string inodenum="";
	dirent tmpDirent ;
    int i=0;
    while(i<buf.length()){
    	while(buf[i]!=','){
    		filename+=buf[i];
    		i++;
    	}
    	i++;
    	while(buf[i]!=','&&i<buf.length()){
    		inodenum+=buf[i];
    		i++;
    	}
    	 tmpDirent.name = filename;
         tmpDirent.inum = n2i(inodenum);
          list.push_back(tmpDirent);     

         filename="";
         inodenum="";
         i++;
    }
    return r;
}

int
yfs_client::read(inum ino, size_t size, off_t off, std::string &data)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: read using ec->get().
     */
    string buf;
    ec->get(ino, buf);
    if(off > buf.length()) 
    	data = "";
	else 
		data = buf.substr(off, size);
    return r;
}

int
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    string buf;
    ec->get(ino, buf);
    int len = buf.size();
     if ((size_t)(off+size) >= len) {
        buf.resize(off+size);
    }
    for (size_t i=off; i<off+size; i++)
        buf[i] = data[i-off];
    bytes_written = size;
    ec->put(ino, buf);
    return r;
}

int yfs_client::unlink(inum parent,const char *name)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
     
    
       inum inodenum = 0;
    bool found = false;
    r = lookup(parent, name, found, inodenum);
    if (r == IOERR)
        return r;
    if(found) {
        string buf;
       ec->get(parent, buf) ;
        ec->remove(inodenum);
        size_t begin_pos = buf.find(name);
        buf.replace(begin_pos, strlen(name)+filename(inodenum).size()+2,"");    
        if (buf[buf.length()-1] == ',')
            buf.replace(buf.length()-1, 1, "");
        ec->put(parent, buf);       
    }
    else
        r = NOENT;  
    return r;
}



