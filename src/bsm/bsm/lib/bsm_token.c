/*
 * Copyright (c) 2004, Apple Computer, Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>

struct socket;

#include <libbsm.h>

#define GET_TOKEN_AREA(t, dptr, length) \
        do {\
                t = (token_t *) malloc (sizeof(token_t));\
                if(t != NULL)\
                {\
                        t->len = length;\
                        t->t_data = (u_char *) malloc (length * sizeof(u_char));\
                        if((dptr = t->t_data) == NULL)\
                        {\
                                free(t);\
                                t = NULL;\
                        }\
                        else\
                        {\
                                memset(dptr, 0, length);\
                        }\
                }\
        }while(0)



/*
 * token ID                1 byte
 * argument #              1 byte
 * argument value          4 bytes/8 bytes (32-bit/64-bit value)
 * text length             2 bytes
 * text                    N bytes + 1 terminating NULL byte
 */
token_t *au_to_arg32(char n, const char *text, u_int32_t v)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t textlen;
	
	if(text == NULL) {
		return NULL;	
	}
	
	/* Make sure that text is null terminated */
	textlen = strlen(text);
	if(text[textlen] != '\0') {	
		return NULL;
	}
	
	GET_TOKEN_AREA(t, dptr, 9 + textlen);
	if(t == NULL) {
		return NULL;
	}

	textlen += 1;

	ADD_U_CHAR(dptr, AU_ARG32_TOKEN);
	ADD_U_CHAR(dptr, n);
	ADD_U_INT32(dptr, v);
	ADD_U_INT16(dptr, textlen);
	ADD_STRING(dptr, text, textlen);

	return t;

}

token_t *au_to_arg64(char n, const char *text, u_int64_t v)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t textlen;
	
	if(text == NULL) {
		return NULL;	
	}
	
	/* Make sure that text is null terminated */
	textlen = strlen(text);
	if(text[textlen] != '\0') {	
		return NULL;
	}
	
	GET_TOKEN_AREA(t, dptr, 13 + textlen);
	if(t == NULL) {
		return NULL;
	}

	textlen += 1;	

	ADD_U_CHAR(dptr, AU_ARG64_TOKEN);
	ADD_U_CHAR(dptr, n);
	ADD_U_INT64(dptr, v);
	ADD_U_INT16(dptr, textlen);
	ADD_STRING(dptr, text, textlen);
	
	return t;

}

token_t *au_to_arg(char n, char *text, u_int32_t v)
{
	return au_to_arg32(n, text, v);
}

#if BSM_BUG_VNODE_VATTR_DEFINED	/* see libbsm.h */
/*
 * token ID                1 byte
 * file access mode        4 bytes
 * owner user ID           4 bytes
 * owner group ID          4 bytes
 * file system ID          8 bytes
 * node ID                 8 bytes
 * device                  4 bytes/8 bytes (32-bit/64-bit)
 */
token_t *au_to_attr32(struct vnode_vattr *attr)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t pad0_16 = 0;
	u_int16_t pad0_32 = 0;

	if(attr == NULL) {
		return NULL;
	}
	
	
	GET_TOKEN_AREA(t, dptr, 29);
	if(t == NULL) {
		return NULL;
	}

	ADD_U_CHAR(dptr, AU_ATTR32_TOKEN);

	/* 
	 * Darwin defines the size for the file mode 
	 * as 2 bytes; BSM defines 4 so pad with 0
	 */    
	ADD_U_INT16(dptr, pad0_16);
	ADD_U_INT16(dptr, attr->va_mode);

	ADD_U_INT32(dptr, attr->va_uid);
	ADD_U_INT32(dptr, attr->va_gid);
	ADD_U_INT32(dptr, attr->va_fsid);

	ADD_U_INT64(dptr, attr->va_fileid);

	ADD_U_INT32(dptr, attr->va_rdev);	/* OK if sizeof(dev_t) unchanged */
	
	return t;
}

token_t *au_to_attr64(struct vnode_vattr *attr)
{
	return NULL;
}

token_t *au_to_attr(struct vnode_vattr *attr)
{
	return au_to_attr32(attr);

}
#endif	/* BSM_BUG_VNODE_VATTR_DEFINED */


/*
 * token ID                1 byte
 * how to print            1 byte
 * basic unit              1 byte
 * unit count              1 byte
 * data items              (depends on basic unit)
 */
token_t *au_to_data(char unit_print, char unit_type,
                    char unit_count, unsigned char *p)
{
	token_t *t;
	u_char *dptr = NULL;
	size_t datasize, totdata;
	
	if(p == NULL) {
		return NULL;
	}
	
	/* Determine the size of the basic unit */
	switch(unit_type) {
		case AUR_BYTE:	datasize = AUR_BYTE_SIZE;
						break;

		case AUR_SHORT:	datasize = AUR_SHORT_SIZE;
						break;
	
		case AUR_LONG:	datasize = AUR_LONG_SIZE;
						break;
				
		default: return NULL;
	}

	totdata = datasize * unit_count;
	
	GET_TOKEN_AREA(t, dptr, totdata + 4);
	if(t == NULL) {
		return NULL;
	}

	ADD_U_CHAR(dptr, AU_ARB_TOKEN);
	ADD_U_CHAR(dptr, unit_print);
	ADD_U_CHAR(dptr, unit_type);
	ADD_U_CHAR(dptr, unit_count);
	ADD_MEM(dptr, p, totdata);
	
	return t;
}


/*
 * token ID                1 byte
 * status		   4 bytes
 * return value            4 bytes  
 */
token_t *au_to_exit(int retval, int err)
{
	token_t *t;
	u_char *dptr = NULL;
		 
	GET_TOKEN_AREA(t, dptr, 9);
	if(t == NULL) {
		return NULL;
	}
	
	ADD_U_CHAR(dptr, AU_EXIT_TOKEN);
	ADD_U_INT32(dptr, err);
	ADD_U_INT32(dptr, retval);

	return t;	
}

/*
 */
token_t *au_to_groups(gid_t *groups)
{
	return au_to_newgroups(MAX_GROUPS, groups);	
}

/*
 * token ID                1 byte
 * number groups           2 bytes
 * group list              count * 4 bytes
 */
token_t *au_to_newgroups(u_int16_t n, gid_t *groups)
{
	token_t *t;
	u_char *dptr = NULL;
	int i;
   	
	if(groups == NULL) {
		return NULL;
	}
	
	GET_TOKEN_AREA(t, dptr, n * 4 + 3);
	if(t == NULL) {
		return NULL;
	}
 
	ADD_U_CHAR(dptr, AU_NEWGROUPS_TOKEN);
	ADD_U_INT16(dptr, n);
	for(i = 0; i < n; i++) {
	    ADD_U_INT32(dptr, groups[i]);
	}
 
	return t;	
}




/*
 * token ID                1 byte
 * internet address        4 bytes
 */
token_t *au_to_in_addr(struct in_addr *internet_addr)
{
	token_t *t;
	u_char *dptr = NULL;
		 
	if(internet_addr == NULL) {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, 5);
	if(t == NULL) {
		return NULL;
	}
	
	ADD_U_CHAR(dptr, AU_IN_ADDR_TOKEN);
	ADD_U_INT32(dptr, internet_addr->s_addr);

	return t;
}

/*
 * token ID                1 byte
 * address type/length     4 bytes
 * Address                16 bytes 
 */
token_t *au_to_in_addr_ex(struct in6_addr *internet_addr)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int32_t type = AF_INET6;
	
	if(internet_addr == NULL) {
		return NULL;
	}
	
	GET_TOKEN_AREA(t, dptr, 21);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_IN_ADDR_EX_TOKEN);
	ADD_U_INT32(dptr, type);
	ADD_U_INT32(dptr, internet_addr->__u6_addr.__u6_addr32[0]);
	ADD_U_INT32(dptr, internet_addr->__u6_addr.__u6_addr32[1]);
	ADD_U_INT32(dptr, internet_addr->__u6_addr.__u6_addr32[2]);
	ADD_U_INT32(dptr, internet_addr->__u6_addr.__u6_addr32[3]);

	return t;
}

/*
 * token ID                1 byte
 * ip header		   20 bytes
 */
token_t *au_to_ip(struct ip *ip)
{
	token_t *t;
	u_char *dptr = NULL;

	if(ip == NULL) {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, 21);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_IP_TOKEN);
	ADD_MEM(dptr, ip, sizeof(struct ip));
				
	return t;
}

/*
 * token ID                1 byte
 * object ID type          1 byte
 * object ID               4 bytes
 */
token_t *au_to_ipc(char type, int id)
{
	token_t *t;
	u_char *dptr = NULL;
		 
		 
	GET_TOKEN_AREA(t, dptr, 6);
	if(t == NULL) {
		return NULL;
	}
				 
	ADD_U_CHAR(dptr, AU_IPC_TOKEN);
	ADD_U_CHAR(dptr, type);
	ADD_U_INT32(dptr, id);

	return t;
}

/*
 * token ID                1 byte
 * owner user ID           4 bytes
 * owner group ID          4 bytes
 * creator user ID         4 bytes
 * creator group ID        4 bytes
 * access mode             4 bytes
 * slot sequence #         4 bytes
 * key                     4 bytes
 */
token_t *au_to_ipc_perm(struct ipc_perm *perm)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t pad0 = 0;
	
	
	if(perm == NULL) {
		return NULL;
	}
	
	GET_TOKEN_AREA(t, dptr, 29);
	if(t == NULL) {
		return NULL;
	}
				
	ADD_U_CHAR(dptr, AU_IPCPERM_TOKEN);


	/* 
	 * Darwin defines the sizes for ipc_perm members
	 * as 2 bytes; BSM defines 4 so pad with 0
	 */    
	ADD_U_INT16(dptr, pad0);
	ADD_U_INT16(dptr, perm->uid);
	
	ADD_U_INT16(dptr, pad0);
	ADD_U_INT16(dptr, perm->gid);
	
	ADD_U_INT16(dptr, pad0);
	ADD_U_INT16(dptr, perm->cuid);

	ADD_U_INT16(dptr, pad0);
	ADD_U_INT16(dptr, perm->cgid);

	ADD_U_INT16(dptr, pad0);
	ADD_U_INT16(dptr, perm->mode);

	ADD_U_INT16(dptr, pad0);
#if defined (__DARWIN_UNIX03) || defined(__LP64__)
	ADD_U_INT16(dptr, perm->_seq);
#else
	ADD_U_INT16(dptr, perm->seq);
#endif

/* XXX  _key (or key) is a _signed_ 32-bit int */
#if defined (__DARWIN_UNIX03) || defined(__LP64__)
	ADD_U_INT32(dptr, perm->_key);
#else
	ADD_U_INT32(dptr, perm->key);
#endif

	return t;
}


/*
 * token ID                1 byte
 * port IP address         2 bytes
 */
token_t *au_to_iport(u_int16_t iport)
{
	token_t *t;
	u_char *dptr = NULL;
		

	GET_TOKEN_AREA(t, dptr, 3);
	if(t == NULL) {
		return NULL;
	}

	ADD_U_CHAR(dptr, AU_IPORT_TOKEN);
	ADD_U_INT16(dptr, iport);
										 
	return t;
}


/*
 * token ID                1 byte
 * size 				   2 bytes
 * data                    size bytes
 */
token_t *au_to_opaque(char *data, u_int16_t bytes)
{
	token_t *t;
	u_char *dptr = NULL;
			 
	if((data == NULL) || (bytes <= 0)) {
		return NULL;
	}
 
	GET_TOKEN_AREA(t, dptr, bytes + 3);
	if(t == NULL) {
		return NULL;
	}
 
	ADD_U_CHAR(dptr, AU_OPAQUE_TOKEN);
	ADD_U_INT16(dptr, bytes);
	ADD_MEM(dptr, data, bytes);
 
	return t;
}

/*
 * token ID                	1 byte
 * seconds of time			4 bytes
 * milliseconds of time		4 bytes
 * file name len			2 bytes
 * file pathname			N bytes + 1 terminating NULL byte   	
 */
token_t *au_to_file(char *file)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t filelen;
	struct timeval tm;
	struct timezone tzp;
	u_int32_t timems;

	if(gettimeofday(&tm, &tzp) == -1) {
		return NULL;
	}
	
	if(file == NULL) {
		return NULL;
	}
	/* Make sure that text is null terminated */
	filelen = strlen(file);
	if(file[filelen] != '\0') {
		return NULL;
	}
 
	GET_TOKEN_AREA(t, dptr, filelen + 12);
	if(t == NULL) {
		return NULL;
	}
		
	filelen += 1;
	timems = tm.tv_usec/1000;				 

	ADD_U_CHAR(dptr, AU_FILE_TOKEN);
	ADD_U_INT32(dptr, tm.tv_sec);
	ADD_U_INT32(dptr, timems); /* We need time in ms */
	ADD_U_INT16(dptr, filelen);
	ADD_STRING(dptr, file, filelen);
	 
	return t;

}


/*
 * token ID                1 byte
 * text length             2 bytes
 * text                    N bytes + 1 terminating NULL byte
 */
token_t *au_to_text(const char *text)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t textlen;
	
	if(text == NULL) {
		return NULL;
	}
	/* Make sure that text is null terminated */
	textlen = strlen(text);
	if(text[textlen] != '\0') {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, textlen + 4);
	if(t == NULL) {
		return NULL;
	}

	textlen += 1;
						 
	ADD_U_CHAR(dptr, AU_TEXT_TOKEN);
	ADD_U_INT16(dptr, textlen);
	ADD_STRING(dptr, text, textlen);

	return t;
}

/*
 * token ID                1 byte
 * path length             2 bytes
 * path                    N bytes + 1 terminating NULL byte
 */
token_t *au_to_path(char *text)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t textlen;
	
	if(text == NULL) {
		return NULL;
	}
	/* Make sure that text is null terminated */
	textlen = strlen(text);
	if(text[textlen] != '\0') {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, textlen + 4);
	if(t == NULL) {
		return NULL;
	}

	textlen += 1;
						 
	ADD_U_CHAR(dptr, AU_PATH_TOKEN);
	ADD_U_INT16(dptr, textlen);
	ADD_STRING(dptr, text, textlen);

	return t;
}

/*
 * token ID                1 byte
 * audit ID                4 bytes
 * effective user ID       4 bytes
 * effective group ID      4 bytes
 * real user ID            4 bytes
 * real group ID           4 bytes
 * process ID              4 bytes
 * session ID              4 bytes
 * terminal ID
 *   port ID               4 bytes/8 bytes (32-bit/64-bit value)
 *   machine address       4 bytes
 */
token_t *au_to_process32(au_id_t auid, uid_t euid, gid_t egid,
		               uid_t ruid, gid_t rgid, pid_t pid,
		               au_asid_t sid, au_tid_t *tid)
{
	token_t *t;
	u_char *dptr = NULL;
	
	if(tid == NULL) {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, 37);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_PROCESS_32_TOKEN);
	ADD_U_INT32(dptr, auid);
	ADD_U_INT32(dptr, euid);
	ADD_U_INT32(dptr, egid);
	ADD_U_INT32(dptr, ruid);
	ADD_U_INT32(dptr, rgid);
	ADD_U_INT32(dptr, pid);
	ADD_U_INT32(dptr, sid);
	ADD_U_INT32(dptr, tid->port);
	ADD_U_INT32(dptr, tid->machine);
	 
	return t;
}

token_t *au_to_process64(au_id_t auid, uid_t euid, gid_t egid,
		               uid_t ruid, gid_t rgid, pid_t pid,
		               au_asid_t sid, au_tid_t *tid)
{
	return NULL;
}

token_t *au_to_process(au_id_t auid, uid_t euid, gid_t egid,
		               uid_t ruid, gid_t rgid, pid_t pid,
		               au_asid_t sid, au_tid_t *tid)
{
	return au_to_process32(auid, euid, egid, ruid, rgid, pid,
			sid, tid);
}

/*
 * token ID                1 byte
 * audit ID                4 bytes
 * effective user ID       4 bytes
 * effective group ID      4 bytes
 * real user ID            4 bytes
 * real group ID           4 bytes
 * process ID              4 bytes
 * session ID              4 bytes
 * terminal ID
 *   port ID               4 bytes/8 bytes (32-bit/64-bit value)
 *   address type-len      4 bytes
 *   machine address      16 bytes
 */
token_t *au_to_process32_ex(au_id_t auid, uid_t euid, gid_t egid,
		               	   uid_t ruid, gid_t rgid, pid_t pid,
		                   au_asid_t sid, au_tid_addr_t *tid)
{
	token_t *t;
	u_char *dptr = NULL;
	
	if(tid == NULL) {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, 53);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_PROCESS_32_EX_TOKEN);
	ADD_U_INT32(dptr, auid);
	ADD_U_INT32(dptr, euid);
	ADD_U_INT32(dptr, egid);
	ADD_U_INT32(dptr, ruid);
	ADD_U_INT32(dptr, rgid);
	ADD_U_INT32(dptr, pid);
	ADD_U_INT32(dptr, sid);
	ADD_U_INT32(dptr, tid->at_port);
	ADD_U_INT32(dptr, tid->at_type);
	ADD_U_INT32(dptr, tid->at_addr[0]);
	ADD_U_INT32(dptr, tid->at_addr[1]);
	ADD_U_INT32(dptr, tid->at_addr[2]);
	ADD_U_INT32(dptr, tid->at_addr[3]);
	 
	return t;
}

token_t *au_to_process64_ex(au_id_t auid, uid_t euid, gid_t egid,
		               	   uid_t ruid, gid_t rgid, pid_t pid,
		                   au_asid_t sid, au_tid_addr_t *tid)
{
	return NULL;
}

token_t *au_to_process_ex(au_id_t auid, uid_t euid, gid_t egid,
		               	   uid_t ruid, gid_t rgid, pid_t pid,
		                   au_asid_t sid, au_tid_addr_t *tid)
{
	return au_to_process32_ex(auid, euid, egid, ruid, rgid, 
			pid, sid, tid);
}

/*
 * token ID                1 byte
 * error status            1 byte
 * return value            4 bytes/8 bytes (32-bit/64-bit value)
 */
token_t *au_to_return32(char status, u_int32_t ret)
{
	token_t *t;
	u_char *dptr = NULL;
	

	GET_TOKEN_AREA(t, dptr, 6);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_RETURN_32_TOKEN);
	ADD_U_CHAR(dptr, status);
	ADD_U_INT32(dptr, ret);

	return t;
}

token_t *au_to_return64(char status, u_int64_t ret)
{
	token_t *t;
	u_char *dptr = NULL;
	

	GET_TOKEN_AREA(t, dptr, 10);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_RETURN_64_TOKEN);
	ADD_U_CHAR(dptr, status);
	ADD_U_INT64(dptr, ret);

	return t;
}

token_t *au_to_return(char status, u_int32_t ret)
{
	return au_to_return32(status, ret);
}

/*
 * token ID                1 byte
 * sequence number         4 bytes
 */
token_t *au_to_seq(u_int32_t audit_count)
{
	token_t *t;
	u_char *dptr = NULL;
	

	GET_TOKEN_AREA(t, dptr, 5);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_SEQ_TOKEN);
	ADD_U_INT32(dptr, audit_count);

	return t;
}

/*
 * token ID                1 byte
 * socket type             2 bytes
 * local port              2 bytes
 * local Internet address  4 bytes
 * remote port             2 bytes
 * remote Internet address 4 bytes
 */
token_t *au_to_socket(struct socket *so) {
	return NULL;
}

/*
 * token ID                1 byte
 * socket type             2 bytes
 * local port              2 bytes
 * address type/length     4 bytes
 * local Internet address  4 bytes/16 bytes (IPv4/IPv6 address)
 * remote port             4 bytes
 * address type/length     4 bytes
 * remote Internet address 4 bytes/16 bytes (IPv4/IPv6 address)
 */
token_t *au_to_socket_ex_32(u_int16_t lp, u_int16_t rp, 
	struct sockaddr *la, struct sockaddr *ra)
{
	return NULL;
}

token_t *au_to_socket_ex_128(u_int16_t lp, u_int16_t rp, 
	struct sockaddr *la, struct sockaddr *ra)
{
	return NULL;
}


/*
 * token ID                1 byte
 * socket family           2 bytes
 * local port              2 bytes
 * socket address          4 bytes
 */
token_t *au_to_sock_inet32(struct sockaddr_in *so)
{
	token_t *t;
	u_char *dptr = NULL;

	if(so == NULL) {
		return NULL;
	}	

	GET_TOKEN_AREA(t, dptr, 9);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_SOCK_INET_32_TOKEN);
	/* 
	 * In Darwin, sin_family is one octet, but BSM defines the token
 	 * to store two. So we copy in a 0 first.
 	 */
	ADD_U_CHAR(dptr, 0);
	ADD_U_CHAR(dptr, so->sin_family);
	ADD_U_INT16(dptr, so->sin_port);
	ADD_U_INT32(dptr, so->sin_addr.s_addr);

	return t;

}

token_t *au_to_sock_inet128(struct sockaddr_in6 *so)
{
	token_t *t;
	u_char *dptr = NULL;

	if(so == NULL) {
		return NULL;
	}	

	GET_TOKEN_AREA(t, dptr, 21);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_SOCK_INET_128_TOKEN);
	/* 
	 * In Darwin, sin6_family is one octet, but BSM defines the token
 	 * to store two. So we copy in a 0 first.
 	 */
	ADD_U_CHAR(dptr, 0);
	ADD_U_CHAR(dptr, so->sin6_family);
		
	ADD_U_INT16(dptr, so->sin6_port);
	ADD_U_INT32(dptr, so->sin6_addr.__u6_addr.__u6_addr32[0]);
	ADD_U_INT32(dptr, so->sin6_addr.__u6_addr.__u6_addr32[1]);
	ADD_U_INT32(dptr, so->sin6_addr.__u6_addr.__u6_addr32[2]);
	ADD_U_INT32(dptr, so->sin6_addr.__u6_addr.__u6_addr32[3]);

	return t;
		
}

token_t *au_to_sock_inet(struct sockaddr_in *so)
{
	return au_to_sock_inet32(so);
}

/*
 * token ID                1 byte
 * audit ID                4 bytes
 * effective user ID       4 bytes
 * effective group ID      4 bytes
 * real user ID            4 bytes
 * real group ID           4 bytes
 * process ID              4 bytes
 * session ID              4 bytes
 * terminal ID
 *   port ID               4 bytes/8 bytes (32-bit/64-bit value)
 *   machine address       4 bytes
 */
token_t *au_to_subject32(au_id_t auid, uid_t euid, gid_t egid,
						uid_t ruid, gid_t rgid, pid_t pid,
						au_asid_t sid, au_tid_t *tid)
{
	token_t *t;
	u_char *dptr = NULL;
	
	if(tid == NULL) {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, 37);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_SUBJECT_32_TOKEN);
	ADD_U_INT32(dptr, auid);
	ADD_U_INT32(dptr, euid);
	ADD_U_INT32(dptr, egid);
	ADD_U_INT32(dptr, ruid);
	ADD_U_INT32(dptr, rgid);
	ADD_U_INT32(dptr, pid);
	ADD_U_INT32(dptr, sid);
	ADD_U_INT32(dptr, tid->port);
	ADD_U_INT32(dptr, tid->machine);
	 
	return t;
}

token_t *au_to_subject64(au_id_t auid, uid_t euid, gid_t egid,
						uid_t ruid, gid_t rgid, pid_t pid,
						au_asid_t sid, au_tid_t *tid)
{
	return NULL;
}

token_t *au_to_subject(au_id_t auid, uid_t euid, gid_t egid,
						uid_t ruid, gid_t rgid, pid_t pid,
						au_asid_t sid, au_tid_t *tid)
{
	return au_to_subject32(auid, euid, egid, ruid, rgid,
			pid, sid, tid);
}

/*
 * token ID                1 byte
 * audit ID                4 bytes
 * effective user ID       4 bytes
 * effective group ID      4 bytes
 * real user ID            4 bytes
 * real group ID           4 bytes
 * process ID              4 bytes
 * session ID              4 bytes
 * terminal ID
 *   port ID               4 bytes/8 bytes (32-bit/64-bit value)
 *   address type/length   4 bytes
 *   machine address      16 bytes
 */
token_t *au_to_subject32_ex(au_id_t auid, uid_t euid,
	                       gid_t egid, uid_t ruid, gid_t rgid, pid_t pid,
		                   au_asid_t sid, au_tid_addr_t *tid)
{
	token_t *t;
	u_char *dptr = NULL;
	
	if(tid == NULL) {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, 53);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_SUBJECT_32_EX_TOKEN);
	ADD_U_INT32(dptr, auid);
	ADD_U_INT32(dptr, euid);
	ADD_U_INT32(dptr, egid);
	ADD_U_INT32(dptr, ruid);
	ADD_U_INT32(dptr, rgid);
	ADD_U_INT32(dptr, pid);
	ADD_U_INT32(dptr, sid);
	ADD_U_INT32(dptr, tid->at_port);
	ADD_U_INT32(dptr, tid->at_type);
	ADD_U_INT32(dptr, tid->at_addr[0]);
	ADD_U_INT32(dptr, tid->at_addr[1]);
	ADD_U_INT32(dptr, tid->at_addr[2]);
	ADD_U_INT32(dptr, tid->at_addr[3]);
	 
	return t;
}

token_t *au_to_subject64_ex(au_id_t auid, uid_t euid,
	                       gid_t egid, uid_t ruid, gid_t rgid, pid_t pid,
		                   au_asid_t sid, au_tid_addr_t *tid)
{
	return NULL;
}

token_t *au_to_subject_ex(au_id_t auid, uid_t euid,
	                       gid_t egid, uid_t ruid, gid_t rgid, pid_t pid,
		                   au_asid_t sid, au_tid_addr_t *tid)
{
	return au_to_subject32_ex(auid, euid, egid, ruid, rgid,
			pid, sid, tid);
}

/*
 * Collects audit information for the current process 
 * and creates a subject token from it
 */
token_t *au_to_me(void)
{

	auditinfo_t auinfo;

	if(getaudit(&auinfo) != 0) {
		return NULL;	
	} 

	return au_to_subject32(auinfo.ai_auid, geteuid(), getegid(),
		getuid(), getgid(), getpid(), auinfo.ai_asid, 
		&auinfo.ai_termid);

}

/*
 * token ID				1 byte
 * count				4 bytes
 * text					count null-terminated strings 
 */
token_t *au_to_exec_args(const char **args)
{
	token_t *t;
	u_char *dptr = NULL;
	const char *nextarg;
	int i, count = 0;
	size_t totlen = 0;
	
	if(args == NULL) {
		return NULL;
	}
	
	nextarg = *args;
	
	while(nextarg != NULL) {
		int nextlen;
	
		nextlen = strlen(nextarg);
		if(nextarg[nextlen] != '\0') {
			return NULL;
		}
		
		totlen += nextlen + 1;
		count++;
		nextarg = *(args + count);
	}
	
	
	GET_TOKEN_AREA(t, dptr, 5 + totlen);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_EXEC_ARG_TOKEN);
	ADD_U_INT32(dptr, count);

	for(i =0; i< count; i++) {
		nextarg = *(args + i);
		ADD_MEM(dptr, nextarg, strlen(nextarg) + 1);
	}
	 
	return t;
}


/*
 * token ID				1 byte
 * count				4 bytes
 * text					count null-terminated strings 
 */
token_t *au_to_exec_env(const char **env)
{
	token_t *t;
	u_char *dptr = NULL;
	int i, count = 0;
	size_t totlen = 0;
	const char *nextenv;
	
	if(env == NULL) {
		return NULL;
	}
	
	nextenv = *env;
	
	while(nextenv != NULL) {
		int nextlen;
		
		nextlen = strlen(nextenv);
		if(nextenv[nextlen] != '\0') {
			return NULL;
		}
		
		totlen += nextlen + 1;
		count++;
		nextenv = *(env + count);
	}
	
	
	GET_TOKEN_AREA(t, dptr, 5 + totlen);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_EXEC_ENV_TOKEN);
	ADD_U_INT32(dptr, count);

	for(i =0; i< count; i++) {
		nextenv = *(env + i);
		ADD_MEM(dptr, nextenv, strlen(nextenv) + 1);
	}
	 
	return t;
}


/*
 * token ID                1 byte
 * record byte count       4 bytes
 * version #               1 byte    [2]
 * event type              2 bytes
 * event modifier          2 bytes
 * seconds of time         4 bytes/8 bytes (32-bit/64-bit value)
 * milliseconds of time    4 bytes/8 bytes (32-bit/64-bit value)
 */
token_t *au_to_header32(int rec_size, au_event_t e_type, au_emod_t e_mod)
{
	token_t *t;
	u_char *dptr = NULL;
	struct timeval tm;
	struct timezone tzp;
	u_int32_t timems;
	
	if(gettimeofday(&tm, &tzp) == -1) {
		return NULL;
	}

	GET_TOKEN_AREA(t, dptr, 18);
	if(t == NULL) {
		return NULL;
	}
	
	ADD_U_CHAR(dptr, AU_HEADER_32_TOKEN);
	ADD_U_INT32(dptr, rec_size);
	ADD_U_CHAR(dptr, HEADER_VERSION);
	ADD_U_INT16(dptr, e_type);
	ADD_U_INT16(dptr, e_mod);

	timems = tm.tv_usec/1000;
	/* Add the timestamp */
	ADD_U_INT32(dptr, tm.tv_sec);
	ADD_U_INT32(dptr, timems); /* We need time in ms */

	return t;
}

token_t *au_to_header64(int rec_size, au_event_t e_type, au_emod_t e_mod)
{
	return NULL;
}

token_t *au_to_header(int rec_size, au_event_t e_type, au_emod_t e_mod)
{
	return au_to_header32(rec_size, e_type, e_mod);
}

/*
 * token ID                1 byte
 * trailer magic number    2 bytes
 * record byte count       4 bytes
 */
token_t *au_to_trailer(int rec_size)
{
	token_t *t;
	u_char *dptr = NULL;
	u_int16_t magic = TRAILER_PAD_MAGIC;
	

	GET_TOKEN_AREA(t, dptr, 7);
	if(t == NULL) {
		return NULL;
	}
						 
	ADD_U_CHAR(dptr, AU_TRAILER_TOKEN);
	ADD_U_INT16(dptr, magic);
	ADD_U_INT32(dptr, rec_size);

	return t;
		
}

