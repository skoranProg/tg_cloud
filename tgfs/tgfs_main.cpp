#include "tgfs.h"
#include "tgfs_data.h"
#include "unistd.h"

extern struct fuse_lowlevel_ops tgfs_opers;

struct tgfs_opts {
    double timeout;
    size_t max_filesize;
};

static const struct fuse_opt tgfs_args[] = {
    {"timeout=%lf", offsetof(struct tgfs_opts, timeout), 0},
    {"timeout=%llu", offsetof(struct tgfs_opts, timeout), 0},
    FUSE_OPT_END};

void tgfs_help() {
    printf("    -o timeout=1.0         Caching timeout\n"
           "    -o max_filesize=1024   Maximal size of one file(in Mb)\n");
}

int make_new_tgfs(int argc, char *argv[], tgfs_net_api *api) {
    int err = 0;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_cmdline_opts opts;
    struct fuse_loop_config config;

    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    if (opts.show_help) {
        printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
        fuse_cmdline_help();
        fuse_lowlevel_help();
        tgfs_help();
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        return 0;
    } else if (opts.show_version) {
        printf("FUSE library version %s\n", fuse_pkgversion());
        fuse_lowlevel_version();
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        return 0;
    }

    if (opts.mountpoint == NULL) {
        printf("usage: %s [options] <mountpoint>\n", argv[0]);
        printf("       %s --help\n", argv[0]);
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        return 1;
    }

    struct tgfs_opts custom_opts = {
        .timeout = 1,
        .max_filesize = 1ll << 30,
    };

    if (fuse_opt_parse(&args, &custom_opts, tgfs_args, NULL) == -1) {
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        return 1;
    }

    if (custom_opts.max_filesize > (1ll << 31)) {
        printf("Files can't be larger than 2 Gb.");
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        return 1;
    }

    if (custom_opts.timeout < 0) {
        printf("Timeout can't be negative");
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        return 1;
    }

    int root_fd = open(opts.mountpoint, O_PATH);
    if (root_fd == -1) {
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        return 1;
    }

    tgfs_data *context = new tgfs_data(opts.debug, custom_opts.timeout, root_fd,
                                       custom_opts.max_filesize, api);

    struct fuse_session *se =
        fuse_session_new(&args, &tgfs_opers, sizeof(tgfs_opers), context);
    if (se == nullptr) {
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        close(root_fd);
        return 1;
    }
    if (fuse_set_signal_handlers(se) != 0) {
        fuse_session_destroy(se);
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        close(root_fd);
        return 1;
    }
    if (fuse_session_mount(se, opts.mountpoint) != 0) {
        fuse_remove_signal_handlers(se);
        fuse_session_destroy(se);
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
        close(root_fd);
        return 1;
    }
    fuse_daemonize(opts.foreground);

    if (opts.singlethread)
        err = fuse_session_loop(se);
    else {
        config.clone_fd = opts.clone_fd;
        config.max_idle_threads = opts.max_idle_threads;
        err = fuse_session_loop_mt(se, &config);
    }

    fuse_session_unmount(se);
    fuse_remove_signal_handlers(se);
    fuse_session_destroy(se);
    free(opts.mountpoint);
    fuse_opt_free_args(&args);
    close(root_fd);
    delete context;

    return err;
}
