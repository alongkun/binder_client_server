/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <linux/types.h>
#include <stdbool.h>
#include <string.h>

#include <private/android_filesystem_config.h>

#include "binder.h"

#define HELLO_SVR_CMD_SAYHELLO     0
#define HELLO_SVR_CMD_SAYHELLO_TO  1
#define HELLO_SVR_CMD_SAYGOODBYE     2
#define HELLO_SVR_CMD_SAYGOODBYE_TO  3

uint32_t svcmgr_lookup(struct binder_state *bs, uint32_t target, const char *name)
{
    uint32_t handle;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE))
        return 0;

    handle = bio_get_ref(&reply);

    if (handle)
        binder_acquire(bs, handle);

    binder_done(bs, &msg, &reply);

    return handle;
}

void sayhello(struct binder_state *bs, uint32_t target)
{
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header

    if (binder_call(bs, &msg, &reply, target, HELLO_SVR_CMD_SAYHELLO))
        return ;

    binder_done(bs, &msg, &reply);
}

uint32_t sayhello_to(struct binder_state *bs, uint32_t target, char *name)
{
    uint32_t handle;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, name);

    if (binder_call(bs, &msg, &reply, target, HELLO_SVR_CMD_SAYHELLO_TO))
        return 0;

    handle = bio_get_uint32(&reply);

//    if (handle)
//        binder_acquire(bs, handle);

    binder_done(bs, &msg, &reply);

    return handle;
}

void saygoodbye(struct binder_state *bs, uint32_t target)
{
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header

    if (binder_call(bs, &msg, &reply, target, HELLO_SVR_CMD_SAYGOODBYE))
        return ;

    binder_done(bs, &msg, &reply);
}

uint32_t saygoodbye_to(struct binder_state *bs, uint32_t target, char *name)
{
    uint32_t handle;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, name);

    if (binder_call(bs, &msg, &reply, target, HELLO_SVR_CMD_SAYGOODBYE_TO))
        return 0;

    handle = bio_get_uint32(&reply);

//    if (handle)
//        binder_acquire(bs, handle);

    binder_done(bs, &msg, &reply);

    return handle;
}

/*
int svcmgr_publish(struct binder_state *bs, uint32_t target, const char *name, void *ptr)
{
    int status;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);
    bio_put_obj(&msg, ptr);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE))
        return -1;

    status = bio_get_uint32(&reply);

    binder_done(bs, &msg, &reply);

    return status;
}
*/

//unsigned token;

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle_hello, handle_goodbye;
    uint32_t ret;

    if(argc < 2)
    {
        fprintf(stderr, "argc err\n");
        return -1;
    }

    bs = binder_open(128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

    handle_hello = svcmgr_lookup(bs, svcmgr, "hello");
    if (!handle_hello)
    {
        fprintf(stderr,"cannot find server hello\n");
        return -1;
    }

    handle_goodbye = svcmgr_lookup(bs, svcmgr, "goodbye");
    if (!handle_goodbye)
    {
        fprintf(stderr,"cannot find server goodbye\n");
        return -1;
    }

    if(!strcmp(argv[1], "hello"))
    {
        if(argc == 2)
        {
            sayhello(bs, handle_hello);
        }
        else if(argc == 3)
        {
            ret = sayhello_to(bs, handle_hello, argv[2]);
            fprintf(stderr, "%s, sayhello_to return %d\n", __FILE__, ret);
        }
    }
    else if(!strcmp(argv[1], "goodbye"))
    {
        if(argc == 2)
        {
            saygoodbye(bs, handle_goodbye);
        }
        else if(argc == 3)
        {
            ret = saygoodbye_to(bs, handle_goodbye, argv[2]);
            fprintf(stderr, "%s, saygoodbye_to return %d\n", __FILE__, ret);
        }
    }

    binder_release(bs, handle_hello);
    binder_release(bs, handle_goodbye);

    /*
    argc--;
    argv++;
    while (argc > 0) {
        if (!strcmp(argv[0],"alt")) {
            handle = svcmgr_lookup(bs, svcmgr, "alt_svc_mgr");
            if (!handle) {
                fprintf(stderr,"cannot find alt_svc_mgr\n");
                return -1;
            }
            svcmgr = handle;
            fprintf(stderr,"svcmgr is via %x\n", handle);
        } else if (!strcmp(argv[0],"lookup")) {
            if (argc < 2) {
                fprintf(stderr,"argument required\n");
                return -1;
            }
            handle = svcmgr_lookup(bs, svcmgr, argv[1]);
            fprintf(stderr,"lookup(%s) = %x\n", argv[1], handle);
            argc--;
            argv++;
        } else if (!strcmp(argv[0],"publish")) {
            if (argc < 2) {
                fprintf(stderr,"argument required\n");
                return -1;
            }
            svcmgr_publish(bs, svcmgr, argv[1], &token);
            argc--;
            argv++;
        } else {
            fprintf(stderr,"unknown command %s\n", argv[0]);
            return -1;
        }
        argc--;
        argv++;
    }
    */


    return 0;
}
