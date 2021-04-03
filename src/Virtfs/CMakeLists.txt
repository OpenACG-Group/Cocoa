add_library(Virtfs STATIC
            Virtfs.h
            VfsIndexNode.h
            VfsIndexNodeTable.cc
            VfsIndexNode.cc VfsDentry.h VfsMountPoint.h VfsBackendDriver.h VfsBackendDriver.cc VfsDentry.cc)
