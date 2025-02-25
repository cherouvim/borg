Positional Arguments and Options: Order matters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Borg only supports taking options (``-s`` and ``--progress`` in the example)
to the left or right of all positional arguments (``repo::archive`` and ``path``
in the example), but not in between them:

::

    borg create -s --progress repo::archive path  # good and preferred
    borg create repo::archive path -s --progress  # also works
    borg create -s repo::archive path --progress  # works, but ugly
    borg create repo::archive -s --progress path  # BAD

This is due to a problem in the argparse module: http://bugs.python.org/issue15112


Repository URLs
~~~~~~~~~~~~~~~

**Local filesystem** (or locally mounted network filesystem):

``/path/to/repo`` - filesystem path to repo directory, absolute path

``path/to/repo`` - filesystem path to repo directory, relative path

Also, stuff like ``~/path/to/repo`` or ``~other/path/to/repo`` works (this is
expanded by your shell).

Note: you may also prepend a ``file://`` to a filesystem path to get URL style.

**Remote repositories** accessed via ssh user@host:

``user@host:/path/to/repo`` - remote repo, absolute path

``ssh://user@host:port/path/to/repo`` - same, alternative syntax, port can be given


**Remote repositories with relative paths** can be given using this syntax:

``user@host:path/to/repo`` - path relative to current directory

``user@host:~/path/to/repo`` - path relative to user's home directory

``user@host:~other/path/to/repo`` - path relative to other's home directory

Note: giving ``user@host:/./path/to/repo`` or ``user@host:/~/path/to/repo`` or
``user@host:/~other/path/to/repo`` is also supported, but not required here.


**Remote repositories with relative paths, alternative syntax with port**:

``ssh://user@host:port/./path/to/repo`` - path relative to current directory

``ssh://user@host:port/~/path/to/repo`` - path relative to user's home directory

``ssh://user@host:port/~other/path/to/repo`` - path relative to other's home directory


If you frequently need the same repo URL, it is a good idea to set the
``BORG_REPO`` environment variable to set a default for the repo URL:

::

    export BORG_REPO='ssh://user@host:port/path/to/repo'

Then just leave away the repo URL if only a repo URL is needed and you want
to use the default - it will be read from BORG_REPO then.

Use ``::`` syntax to give the repo URL when syntax requires giving a positional
argument for the repo (e.g. ``borg mount :: /mnt``).


Repository / Archive Locations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Many commands want either a repository (just give the repo URL, see above) or
an archive location, which is a repo URL followed by ``::archive_name``.

Archive names must not contain the ``/`` (slash) character. For simplicity,
maybe also avoid blanks or other characters that have special meaning on the
shell or in a filesystem (borg mount will use the archive name as directory
name).

If you have set BORG_REPO (see above) and an archive location is needed, use
``::archive_name`` - the repo URL part is then read from BORG_REPO.


Logging
~~~~~~~

Borg writes all log output to stderr by default. But please note that something
showing up on stderr does *not* indicate an error condition just because it is
on stderr. Please check the log levels of the messages and the return code of
borg for determining error, warning or success conditions.

If you want to capture the log output to a file, just redirect it:

::

    borg create repo::archive myfiles 2>> logfile


Custom logging configurations can be implemented via BORG_LOGGING_CONF.

The log level of the builtin logging configuration defaults to WARNING.
This is because we want Borg to be mostly silent and only output
warnings, errors and critical messages, unless output has been requested
by supplying an option that implies output (e.g. ``--list`` or ``--progress``).

Log levels: DEBUG < INFO < WARNING < ERROR < CRITICAL

Use ``--debug`` to set DEBUG log level -
to get debug, info, warning, error and critical level output.

Use ``--info`` (or ``-v`` or ``--verbose``) to set INFO log level -
to get info, warning, error and critical level output.

Use ``--warning`` (default) to set WARNING log level -
to get warning, error and critical level output.

Use ``--error`` to set ERROR log level -
to get error and critical level output.

Use ``--critical`` to set CRITICAL log level -
to get critical level output.

While you can set misc. log levels, do not expect that every command will
give different output on different log levels - it's just a possibility.

.. warning:: Options ``--critical`` and ``--error`` are provided for completeness,
             their usage is not recommended as you might miss important information.

Return codes
~~~~~~~~~~~~

Borg can exit with the following return codes (rc):

=========== =======
Return code Meaning
=========== =======
0           success (logged as INFO)
1           warning (operation reached its normal end, but there were warnings --
            you should check the log, logged as WARNING)
2           error (like a fatal error, a local or remote exception, the operation
            did not reach its normal end, logged as ERROR)
128+N       killed by signal N (e.g. 137 == kill -9)
=========== =======

If you use ``--show-rc``, the return code is also logged at the indicated
level as the last log entry.

.. _env_vars:

Environment Variables
~~~~~~~~~~~~~~~~~~~~~

Borg uses some environment variables for automation:

General:
    BORG_REPO
        When set, use the value to give the default repository location. If a command needs an archive
        parameter, you can abbreviate as ``::archive``. If a command needs a repository parameter, you
        can either leave it away or abbreviate as ``::``, if a positional parameter is required.
    BORG_PASSPHRASE
        When set, use the value to answer the passphrase question for encrypted repositories.
        It is used when a passphrase is needed to access an encrypted repo as well as when a new
        passphrase should be initially set when initializing an encrypted repo.
        See also BORG_NEW_PASSPHRASE.
    BORG_PASSCOMMAND
        When set, use the standard output of the command (trailing newlines are stripped) to answer the
        passphrase question for encrypted repositories.
        It is used when a passphrase is needed to access an encrypted repo as well as when a new
        passphrase should be initially set when initializing an encrypted repo. Note that the command
        is executed without a shell. So variables, like ``$HOME`` will work, but ``~`` won't.
        If BORG_PASSPHRASE is also set, it takes precedence.
        See also BORG_NEW_PASSPHRASE.
    BORG_PASSPHRASE_FD
        When set, specifies a file descriptor to read a passphrase
        from. Programs starting borg may choose to open an anonymous pipe
        and use it to pass a passphrase. This is safer than passing via
        BORG_PASSPHRASE, because on some systems (e.g. Linux) environment
        can be examined by other processes.
        If BORG_PASSPHRASE or BORG_PASSCOMMAND are also set, they take precedence.
    BORG_NEW_PASSPHRASE
        When set, use the value to answer the passphrase question when a **new** passphrase is asked for.
        This variable is checked first. If it is not set, BORG_PASSPHRASE and BORG_PASSCOMMAND will also
        be checked.
        Main usecase for this is to fully automate ``borg change-passphrase``.
    BORG_DISPLAY_PASSPHRASE
        When set, use the value to answer the "display the passphrase for verification" question when defining a new passphrase for encrypted repositories.
    BORG_HOSTNAME_IS_UNIQUE=no
        Borg assumes that it can derive a unique hostname / identity (see ``borg debug info``).
        If this is not the case or you do not want Borg to automatically remove stale locks,
        set this to *no*.
    BORG_HOST_ID
        Borg usually computes a host id from the FQDN plus the results of ``uuid.getnode()`` (which usually returns
        a unique id based on the MAC address of the network interface. Except if that MAC happens to be all-zero - in
        that case it returns a random value, which is not what we want (because it kills automatic stale lock removal).
        So, if you have a all-zero MAC address or other reasons to better externally control the host id, just set this
        environment variable to a unique value. If all your FQDNs are unique, you can just use the FQDN. If not,
        use fqdn@uniqueid.
    BORG_LOGGING_CONF
        When set, use the given filename as INI_-style logging configuration.
        A basic example conf can be found at ``docs/misc/logging.conf``.
    BORG_RSH
        When set, use this command instead of ``ssh``. This can be used to specify ssh options, such as
        a custom identity file ``ssh -i /path/to/private/key``. See ``man ssh`` for other options. Using
        the ``--rsh CMD`` commandline option overrides the environment variable.
    BORG_REMOTE_PATH
        When set, use the given path as borg executable on the remote (defaults to "borg" if unset).
        Using ``--remote-path PATH`` commandline option overrides the environment variable.
    BORG_FILES_CACHE_TTL
        When set to a numeric value, this determines the maximum "time to live" for the files cache
        entries (default: 20). The files cache is used to quickly determine whether a file is unchanged.
        The FAQ explains this more detailed in: :ref:`always_chunking`
    BORG_SHOW_SYSINFO
        When set to no (default: yes), system information (like OS, Python version, ...) in
        exceptions is not shown.
        Please only use for good reasons as it makes issues harder to analyze.
    BORG_WORKAROUNDS
        A list of comma separated strings that trigger workarounds in borg,
        e.g. to work around bugs in other software.

        Currently known strings are:

        basesyncfile
            Use the more simple BaseSyncFile code to avoid issues with sync_file_range.
            You might need this to run borg on WSL (Windows Subsystem for Linux) or
            in systemd.nspawn containers on some architectures (e.g. ARM).
            Using this does not affect data safety, but might result in a more bursty
            write to disk behaviour (not continuously streaming to disk).
    TMPDIR
        where temporary files are stored (might need a lot of temporary space for some operations), see tempfile_ for details

Some automatic "answerers" (if set, they automatically answer confirmation questions):
    BORG_UNKNOWN_UNENCRYPTED_REPO_ACCESS_IS_OK=no (or =yes)
        For "Warning: Attempting to access a previously unknown unencrypted repository"
    BORG_RELOCATED_REPO_ACCESS_IS_OK=no (or =yes)
        For "Warning: The repository at location ... was previously located at ..."
    BORG_CHECK_I_KNOW_WHAT_I_AM_DOING=NO (or =YES)
        For "Warning: 'check --repair' is an experimental feature that might result in data loss."
    BORG_DELETE_I_KNOW_WHAT_I_AM_DOING=NO (or =YES)
        For "You requested to completely DELETE the repository *including* all archives it contains:"
    BORG_RECREATE_I_KNOW_WHAT_I_AM_DOING=NO (or =YES)
        For "recreate is an experimental feature."

    Note: answers are case sensitive. setting an invalid answer value might either give the default
    answer or ask you interactively, depending on whether retries are allowed (they by default are
    allowed). So please test your scripts interactively before making them a non-interactive script.

Directories and files:
    BORG_BASE_DIR
        Default to '$HOME', '~$USER', '~' (in that order)'.
        If we refer to ~ below, we in fact mean BORG_BASE_DIR.
    BORG_CONFIG_DIR
        Default to '~/.config/borg'. This directory contains the whole config directories.
    BORG_CACHE_DIR
        Default to '~/.cache/borg'. This directory contains the local cache and might need a lot
        of space for dealing with big repositories.
    BORG_SECURITY_DIR
        Default to '~/.config/borg/security'. This directory contains information borg uses to
        track its usage of NONCES ("numbers used once" - usually in encryption context) and other
        security relevant data.
    BORG_KEYS_DIR
        Default to '~/.config/borg/keys'. This directory contains keys for encrypted repositories.
    BORG_KEY_FILE
        When set, use the given filename as repository key file.

Building:
    BORG_OPENSSL_PREFIX
        Adds given OpenSSL header file directory to the default locations (setup.py).
    BORG_LIBLZ4_PREFIX
        Adds given prefix directory to the default locations. If a 'include/lz4.h' is found Borg
        will be linked against the system liblz4 instead of a bundled implementation. (setup.py)
    BORG_LIBB2_PREFIX
        Adds given prefix directory to the default locations. If a 'include/blake2.h' is found Borg
        will be linked against the system libb2 instead of a bundled implementation. (setup.py)
    BORG_LIBZSTD_PREFIX
        Adds given prefix directory to the default locations. If a 'include/zstd.h' is found Borg
        will be linked against the system libzstd instead of a bundled implementation. (setup.py)


Please note:

- be very careful when using the "yes" sayers, the warnings with prompt exist for your / your data's security/safety
- also be very careful when putting your passphrase into a script, make sure it has appropriate file permissions
  (e.g. mode 600, root:root).


.. _INI: https://docs.python.org/3/library/logging.config.html#configuration-file-format

.. _tempfile: https://docs.python.org/3/library/tempfile.html#tempfile.gettempdir

.. _file-systems:

File systems
~~~~~~~~~~~~

We strongly recommend against using Borg (or any other database-like
software) on non-journaling file systems like FAT, since it is not
possible to assume any consistency in case of power failures (or a
sudden disconnect of an external drive or similar failures).

While Borg uses a data store that is resilient against these failures
when used on journaling file systems, it is not possible to guarantee
this with some hardware -- independent of the software used. We don't
know a list of affected hardware.

If you are suspicious whether your Borg repository is still consistent
and readable after one of the failures mentioned above occurred, run
``borg check --verify-data`` to make sure it is consistent.

.. rubric:: Requirements for Borg repository file systems

- Long file names
- At least three directory levels with short names
- Typically, file sizes up to a few hundred MB.
  Large repositories may require large files (>2 GB).
- Up to 1000 files per directory (10000 for repositories initialized with Borg 1.0)
- mkdir(2) should be atomic, since it is used for locking
- Hardlinks are needed for :ref:`borg_upgrade` ``--inplace``

Units
~~~~~

To display quantities, Borg takes care of respecting the
usual conventions of scale. Disk sizes are displayed in `decimal
<https://en.wikipedia.org/wiki/Decimal>`_, using powers of ten (so
``kB`` means 1000 bytes). For memory usage, `binary prefixes
<https://en.wikipedia.org/wiki/Binary_prefix>`_ are used, and are
indicated using the `IEC binary prefixes
<https://en.wikipedia.org/wiki/IEC_80000-13#Prefixes_for_binary_multiples>`_,
using powers of two (so ``KiB`` means 1024 bytes).

Date and Time
~~~~~~~~~~~~~

We format date and time conforming to ISO-8601, that is: YYYY-MM-DD and
HH:MM:SS (24h clock).

For more information about that, see: https://xkcd.com/1179/

Unless otherwise noted, we display local date and time.
Internally, we store and process date and time as UTC.

Resource Usage
~~~~~~~~~~~~~~

Borg might use a lot of resources depending on the size of the data set it is dealing with.

If one uses Borg in a client/server way (with a ssh: repository),
the resource usage occurs in part on the client and in another part on the
server.

If one uses Borg as a single process (with a filesystem repo),
all the resource usage occurs in that one process, so just add up client +
server to get the approximate resource usage.

CPU client:
    - **borg create:** does chunking, hashing, compression, crypto (high CPU usage)
    - **chunks cache sync:** quite heavy on CPU, doing lots of hashtable operations.
    - **borg extract:** crypto, decompression (medium to high CPU usage)
    - **borg check:** similar to extract, but depends on options given.
    - **borg prune / borg delete archive:** low to medium CPU usage
    - **borg delete repo:** done on the server

    It won't go beyond 100% of 1 core as the code is currently single-threaded.
    Especially higher zlib and lzma compression levels use significant amounts
    of CPU cycles. Crypto might be cheap on the CPU (if hardware accelerated) or
    expensive (if not).

CPU server:
    It usually doesn't need much CPU, it just deals with the key/value store
    (repository) and uses the repository index for that.

    borg check: the repository check computes the checksums of all chunks
    (medium CPU usage)
    borg delete repo: low CPU usage

CPU (only for client/server operation):
    When using borg in a client/server way with a ssh:-type repo, the ssh
    processes used for the transport layer will need some CPU on the client and
    on the server due to the crypto they are doing - esp. if you are pumping
    big amounts of data.

Memory (RAM) client:
    The chunks index and the files index are read into memory for performance
    reasons. Might need big amounts of memory (see below).
    Compression, esp. lzma compression with high levels might need substantial
    amounts of memory.

Memory (RAM) server:
    The server process will load the repository index into memory. Might need
    considerable amounts of memory, but less than on the client (see below).

Chunks index (client only):
    Proportional to the amount of data chunks in your repo. Lots of chunks
    in your repo imply a big chunks index.
    It is possible to tweak the chunker params (see create options).

Files index (client only):
    Proportional to the amount of files in your last backups. Can be switched
    off (see create options), but next backup might be much slower if you do.
    The speed benefit of using the files cache is proportional to file size.

Repository index (server only):
    Proportional to the amount of data chunks in your repo. Lots of chunks
    in your repo imply a big repository index.
    It is possible to tweak the chunker params (see create options) to
    influence the amount of chunks being created.

Temporary files (client):
    Reading data and metadata from a FUSE mounted repository will consume up to
    the size of all deduplicated, small chunks in the repository. Big chunks
    won't be locally cached.

Temporary files (server):
    A non-trivial amount of data will be stored on the remote temp directory
    for each client that connects to it. For some remotes, this can fill the
    default temporary directory at /tmp. This can be remediated by ensuring the
    $TMPDIR, $TEMP, or $TMP environment variable is properly set for the sshd
    process.
    For some OSes, this can be done just by setting the correct value in the
    .bashrc (or equivalent login config file for other shells), however in
    other cases it may be necessary to first enable ``PermitUserEnvironment yes``
    in your ``sshd_config`` file, then add ``environment="TMPDIR=/my/big/tmpdir"``
    at the start of the public key to be used in the ``authorized_hosts`` file.

Cache files (client only):
    Contains the chunks index and files index (plus a collection of single-
    archive chunk indexes which might need huge amounts of disk space,
    depending on archive count and size - see FAQ about how to reduce).

Network (only for client/server operation):
    If your repository is remote, all deduplicated (and optionally compressed/
    encrypted) data of course has to go over the connection (``ssh://`` repo url).
    If you use a locally mounted network filesystem, additionally some copy
    operations used for transaction support also go over the connection. If
    you backup multiple sources to one target repository, additional traffic
    happens for cache resynchronization.

.. _platforms:

Support for file metadata
~~~~~~~~~~~~~~~~~~~~~~~~~

Besides regular file and directory structures, Borg can preserve

* symlinks (stored as symlink, the symlink is not followed)
* special files:

  * character and block device files (restored via mknod)
  * FIFOs ("named pipes")
  * special file *contents* can be backed up in ``--read-special`` mode.
    By default the metadata to create them with mknod(2), mkfifo(2) etc. is stored.
* hardlinked regular files, devices, FIFOs (considering all items in the same archive)
* timestamps in nanosecond precision: mtime, atime, ctime
* other timestamps: birthtime (on platforms supporting it)
* permissions:

  * IDs of owning user and owning group
  * names of owning user and owning group (if the IDs can be resolved)
  * Unix Mode/Permissions (u/g/o permissions, suid, sgid, sticky)

On some platforms additional features are supported:

.. Yes/No's are grouped by reason/mechanism/reference.

+-------------------------+----------+-----------+------------+
| Platform                | ACLs     | xattr     | Flags      |
|                         | [#acls]_ | [#xattr]_ | [#flags]_  |
+=========================+==========+===========+============+
| Linux                   | Yes      | Yes       | Yes [1]_   |
+-------------------------+----------+-----------+------------+
| Mac OS X                | Yes      | Yes       | Yes (all)  |
+-------------------------+----------+-----------+------------+
| FreeBSD                 | Yes      | Yes       | Yes (all)  |
+-------------------------+----------+-----------+------------+
| OpenBSD                 | n/a      | n/a       | Yes (all)  |
+-------------------------+----------+-----------+------------+
| NetBSD                  | n/a      | No [2]_   | Yes (all)  |
+-------------------------+----------+-----------+------------+
| Solaris and derivatives | No [3]_  | No [3]_   | n/a        |
+-------------------------+----------+-----------+------------+
| Windows (cygwin)        | No [4]_  | No        | No         |
+-------------------------+----------+-----------+------------+

Other Unix-like operating systems may work as well, but have not been tested at all.

Note that most of the platform-dependent features also depend on the file system.
For example, ntfs-3g on Linux isn't able to convey NTFS ACLs.

.. [1] Only "nodump", "immutable", "compressed" and "append" are supported.
    Feature request :issue:`618` for more flags.
.. [2] Feature request :issue:`1332`
.. [3] Feature request :issue:`1337`
.. [4] Cygwin tries to map NTFS ACLs to permissions with varying degress of success.

.. [#acls] The native access control list mechanism of the OS. This normally limits access to
    non-native ACLs. For example, NTFS ACLs aren't completely accessible on Linux with ntfs-3g.
.. [#xattr] extended attributes; key-value pairs attached to a file, mainly used by the OS.
    This includes resource forks on Mac OS X.
.. [#flags] aka *BSD flags*. The Linux set of flags [1]_ is portable across platforms.
    The BSDs define additional flags.
