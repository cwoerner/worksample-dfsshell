# Simple stacking shell.

See https://github.com/cwoerner/worksample-dfsshell

Maintains state for multiple shell environments, a bit like a *very* poor
man's tmux.  The shell environments are stacked together, enabling you to
push and pop new contexts onto each other.

dfsshell reads from a dfs.yaml file which defines various filesystems to
which it will present a remote shell.

    $ cat dfs.yaml:
    local:
      url: file:///
      desc: the local filesystem

This is a degenerate form of the project, which I started on my own time
and eventually became integrated with an employer's distributed filesystem
offerring.  Unfortunately the full blown implementation is proprietary now.
In this form, dfsshell has native bindings to both qfs and hdfs (via jni
integration) for remote filesystem shell capabilities.  In that version
there are drivers written in C implementing a filesystem operations struct
(dfs_fops) containing members to all the io "syscalls" such as read, write,
seek, among other lower-level interface methods.  The remote fs driver's
syscalls are leveraged by the common built-in functions whose implementation
is filesystem agnostic.  That is, the `cat` builtin is written in terms of
of the abstract dfs_fops interface, and therefor delegates to the syscalls
corresponding to the specific implementation offered by the current dfs
context's dfs_fops struct interface methods.  The dfs context maintains a
pointer to the corresponding fs_ops struct providing appropriate bindings
to the specific flavor of remote filesystem (e.g. qfs, hdfs, etc.).

## Usage

    $ ./dfsshell 

    file:/Users/me> ls
    Desktop		Documents	Downloads	Library		Movies		Music		Pictures	Public		git
    file:/Users/me> dfsstack
    * 0 Fri Sep 13 12:26:21 PDT 2024 file:/Users/me

    file:/Users/me> dfsls
    * 0 file	file:///	local
      1 hdfs-local	hdfs://127.0.0.1:9000	local hdfs

    file:/Users/me> dfs hdfs-local
    hdfs-local:/Users/me> dfsstack
    * 0 Fri Sep 13 12:37:37 PDT 2024 hdfs-local:/Users/me
      1 Fri Sep 13 12:37:26 PDT 2024 file:/Users/me

    hdfs-local:/Users/me> dfspop 
    file:/Users/me> dfsstack
    * 0 Fri Sep 13 12:37:26 PDT 2024 file:/Users/me
    
    file:/Users/me> help
    cd		Change to directory DIR.
    pwd		Show current working directory.
    rename		Rename a file within the current dfs.
    cat		Print the contents of the file to stdout (`cat -text <path>', `cat <path>').
    exit		Quit using dfssh.
    quit		Alias for exit.
    help		Display this text.
    ?		Alias for help.
    ls		List files in current directory.
    dfs		Open a new subshell for the given file system. (`dfs [dfs name]' or `dfs [-ls|-stack|-pop]'.
    dfsls		Show the list of valid dfs (alias for `dfs -ls').
    dfsstack		Show the stack of currently opened dfs sessions (alias for `dfs -stack').
    dfspop		Alias for exit - exit the current dfs context (alias for `dfs -pop').


## Building

    $ git clone git@github.com:cwoerner/worksample-dfsshell.git
    $ cd ./worksample-dfsshell
    $ make dfsshell CPPFLAGS='-DDFS_YAML=\"/opt/dfsshell/dfsshell/dfs.yaml\"'
    
This produces a single binary ./dfsshell, along with object files in ./obj


## Debugging

Default build instructions produce debug symbols. To debug, use your platform's
debugging tool.

On Mac OS X you can use lldb:
    
    $ lldb ./dfsshell
