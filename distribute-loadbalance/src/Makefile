all:
	rm -rf test_*
	gcc -g -std=gnu99  -DCONF_TEST  -lyaml util.h util.c log.h log.c array.h array.c cstring.h cstring.c conf.h conf.c -o test_conf
	gcc -g -std=gnu99  -DTEST  -lyaml  -lpthread util.h util.c log.h log.c array.h array.c cstring.h cstring.c conf.h conf.c dict.h dict.c hashkit.h hashkit.c \
	service_node.h service_node.c schedule_meta.h schedule_meta.c cluster_manager.h cluster_manager.c  -o test_cluster

