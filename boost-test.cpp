#include <stdio.h>
#include <string.h>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>

#define eprintf(fmt, ...) do { \
	fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
	fprintf(stderr, fmt, __VA_ARGS__);\
	fprintf(stderr, "\n"); } while(0)

/**
 *   Character constant for wildcard representing any one character
 *     (SQL style).
 *     */
const char wild_one = '_';

/**
 *   Character constant for wildcard representing zero or more
 *     characters (SQL style).
 *     */
const char wild_many = '%';

class ACL_HOST_AND_IP {
	char *hostname;
	size_t hostname_length;
	long ip, ip_mask;	// Used with masked ip:s

	const char *calc_ip(const char *ip_arg, long *val, char end);

public:
	const char *get_host() const {
		return hostname;
	}

	size_t get_host_len() const {
		return hostname_length;
	}

	bool has_wildcard() {
		return (strchr(hostname, wild_many) ||
			strchr(hostname, wild_one) || ip_mask);
	}

	bool check_allow_all_hosts() {
		return (!hostname ||
			(hostname[0] == wild_many && !hostname[1]));
	}

	void update_hostname(const char *host_arg);

	bool compare_hostname(const char *host_arg, const char *ip_arg);

};

class ACL_ACCESS {
public:
	ACL_HOST_AND_IP host;
	ulong sort;
	ulong access;
};

class ACL_USER:public ACL_ACCESS {
public:
	char *user;
};


static char *str2int(const char *src, int radix, long int lower,
	      long int upper, long int *val)
{
	char *endptr;

	*val = strtol(src, &endptr, radix);

	if (*val < lower) {
		*val = lower;
		return NULL;
	}
	if (*val > upper) {
		*val = upper;
		return NULL;
	}
	if (endptr != '\0') {
		return NULL;
	}
	return (char *)src;
}


const char *
ACL_HOST_AND_IP::calc_ip(const char *ip_arg, long *val, char end)
{
	long ip_val,tmp;
	if (!(ip_arg=str2int(ip_arg,10,0,255,&ip_val)) || *ip_arg != '.')
		return 0;
	ip_val<<=24;
	if (!(ip_arg=str2int(ip_arg+1,10,0,255,&tmp)) || *ip_arg != '.')
		return 0;
	ip_val+=tmp<<16;
	if (!(ip_arg=str2int(ip_arg+1,10,0,255,&tmp)) || *ip_arg != '.')
		return 0;
	ip_val+=tmp<<8;
	if (!(ip_arg=str2int(ip_arg+1,10,0,255,&tmp)) || *ip_arg != end)
		return 0;
	*val=ip_val+tmp;
	return ip_arg;
}


/**
 *   @brief Update the hostname. Updates ip and ip_mask accordingly.
 *
 *     @param host_arg Value to be stored
 *      */
void
ACL_HOST_AND_IP::update_hostname(const char *host_arg)
{
  hostname=(char*) host_arg;     // This will not be modified!
  hostname_length= hostname ? strlen( hostname ) : 0;
  if (!host_arg ||
      (!(host_arg=(char*) calc_ip(host_arg,&ip,'/')) ||
       !(host_arg=(char*) calc_ip(host_arg+1,&ip_mask,'\0'))))
  {
    ip= ip_mask=0;               // Not a masked ip
  }
}

typedef boost::unordered_multimap<char *, boost::unordered_multimap<ACL_HOST_AND_IP *, ACL_USER *> > user_host_map;
user_host_map *uhmap= NULL;

int main(int argc, char **argv)
{
	const char *name;
	const char *host;

	name = argc > 1 ? argv[1] : "user";
	host = argc > 2 ? argv[2] : "127.0.0.1";

	uhmap = new user_host_map();

	ACL_USER *user = new ACL_USER();
	if (!user) {
		eprintf("OOM ACL_USER? size==%llu",
			(unsigned long long)sizeof(ACL_USER));
		return EXIT_FAILURE;
	}
	user->user= strdup(name);
	if (!user->user) {
		eprintf("OOM name strdup(%s)? size==%llu", name,
			(unsigned long long)strlen(name));
		return EXIT_FAILURE;
	}

	user->host.update_hostname(host);
	printf("'%s'@'%s'\n", user->user, user->host.get_host());

	char *name_key= strdup(user->user);
	ACL_HOST_AND_IP *host_key= new ACL_HOST_AND_IP();
	host_key->update_hostname(host);

/*
	*(uhmap)[name_key][host_key]= user;

	ACL_USER *tmp_usr = *(uhmap)[name_key][host_key];

	printf("'%s'@'%s'\n", tmp_usr->user, tmp_usr->host.get_host());
*/
	return 0;
}
