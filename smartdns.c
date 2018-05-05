#include "fast_ping.h"
#include "dns_client.h"
#include "dns_server.h"
#include "hashtable.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

void smartdns_ping_result(const char *host, FAST_PING_RESULT result, int seqno, struct timeval *tv, void *userptr)
{
    if (result == PING_RESULT_RESPONSE) {
        double rtt = tv->tv_sec * 1000.0 + tv->tv_usec / 1000.0;
        printf("%16s: seq=%d time=%.3f\n", host, seqno, rtt);
    } else if (result == PING_RESULT_TIMEOUT) {
		printf("%16s: seq=%d timeout\n", host, seqno);
    }
}

int smartdns_init()
{
    int ret;

    ret = fast_ping_init();
    if (ret != 0) {
        fprintf(stderr, "start ping failed.\n");
        goto errout;
    }

    fast_ping_result_callback(smartdns_ping_result);

    ret = dns_server_init();
    if (ret != 0) {
        fprintf(stderr, "start dns server failed.\n");
        goto errout;
    }

    ret = dns_client_init();
    if (ret != 0) {
        fprintf(stderr, "start dns client failed.\n");
        goto errout;
    }

    return 0;
errout:

    return -1;
}

int smartdns_run()
{
    return dns_server_run();
}

void smartdns_exit()
{
    fast_ping_exit();

    dns_client_exit();

    dns_server_exit();
}

struct data {
    struct list_head list;
    int n;
};

void list_test()
{
    struct list_head head;
    struct list_head *iter;
    int i = 0;

    INIT_LIST_HEAD(&head);

    for (i = 0; i < 10; i++) {
        struct data *h = malloc(sizeof(struct data));
        h->n = i;
        list_add(&h->list, &head);
    }

    list_for_each(iter, &head)
    {
        struct data *d = list_entry(iter, struct data, list);
        printf("%d\n", d->n);
    }
}

struct data_hash {
    struct hlist_node node;
    int n;
    char str[32];
};

int hash_test()
{
    DEFINE_HASHTABLE(ht, 7);
    struct data_hash *temp;
    struct data_hash *obj;
    int i;
    int key;

    for (i = 11; i < 17; i++) {
        temp = malloc(sizeof(struct data_hash));
        temp->n = i * i;
        hash_add(ht, &temp->node, temp->n);
    }

    for (i = 11; i < 17; i++) {
        key = i * i;
        hash_for_each_possible(ht, obj, node, key)
        {
            printf("value: %d\n", obj->n);
        };
    }

    return 0;
}

int hash_string_test()
{
    DEFINE_HASHTABLE(ht, 7);
    struct data_hash *temp;
    struct data_hash *obj;
    int i;
    int key;

    for (i = 0; i < 10; i++) {
        temp = malloc(sizeof(struct data_hash));
        sprintf(temp->str, "%d", i);
        hash_add(ht, &temp->node, hash_string(temp->str));
    }

    for (i = 0; i < 10; i++) {
        char key_str[32];
        sprintf(key_str, "%d", i);
        key = hash_string(key_str);
        hash_for_each_possible(ht, obj, node, key)
        {
            printf("i = %d value: %s\n", i, obj->str);
        };
    }

    return 0;
}

#if 0
struct data_rbtree {
	struct rb_node list;
	int value;
};

int rbtree_test()
{
	struct rb_root root;
	struct rb_node *n;
	RB_EMPTY_ROOT(&root);
	int i;

	for (i = 0; i < 10; i++)
	{
		struct data_rbtree *r = malloc(sizeof(struct data_rbtree));
		r->value = i;
		rb_insert(&r->list, &root);
	}

	n = rb_first(&root);
	int num = 5;
	while (n) {
		struct data_rbtree *r = container_of(n, struct data_rbtree, list);
		if (r->value < num) {
			n = n->rb_left;
		} else if (r->value > num) {
			n = n->rb_right;
		} else {
			printf("n = %d\n", r->value);
			break;
		}
	}

	return 0;
}
#endif

int main(int argc, char *argv[])
{
    int ret;

    atexit(smartdns_exit);

    ret = smartdns_init();
    if (ret != 0) {
        fprintf(stderr, "init smartdns failed.\n");
        goto errout;
    }

    return smartdns_run();

errout:

    return 1;
}