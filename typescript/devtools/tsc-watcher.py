#!/usr/bin/env python3

import os
from posixpath import abspath
import sys
import json
import subprocess
import packages.pyinotify as inotify
import logging, coloredlogs

coloredlogs.install(fmt='%(asctime)s %(hostname)s %(levelname)s %(message)s')

abs_path = os.path.abspath('.')
if len(sys.argv) >= 2:
    abs_path = os.path.abspath(sys.argv[1])

tsconfig_path = os.path.normpath(abs_path + '/tsconfig.json')

logging.info('Using TSConfig file ' + tsconfig_path)

wds = dict()
watcher_manager = inotify.WatchManager()

scheduled_recompile = False
monitoring_list = set()

def append_watch_list(path):
    ret = watcher_manager.add_watch(path, inotify.IN_CLOSE_WRITE|inotify.IN_DELETE)
    if ret[path] < 0:
        return
    wds[path] = ret[path]

def remove_from_watch_list(path):
    if path not in wds:
        raise RuntimeError('Bad path name to remove from monitoring list')
    watcher_manager.rm_watch(wds[path])
    wds.pop(path)

append_watch_list(tsconfig_path)

def update_ts_config():
    new_monitoring_list = set()
    with open(tsconfig_path) as fp:
        config = json.load(fp)
    try:
        files = config['files']
        for f in files:
            p = os.path.normpath(abs_path + '/' + f)
            new_monitoring_list.add(p)
            monitoring_list.add(p)
            append_watch_list(p)
            logging.info('Monitoring file ' + p)
    except KeyError:
        logging.error('tsconfig.json does not specify files')
        return

    for removed in new_monitoring_list ^ monitoring_list:
        logging.info('File ' + removed + ' was removed in monitoring list')
        remove_from_watch_list(removed)


class FileEventHandler(inotify.ProcessEvent):
    def __init__(self, pevent=None, **kargs):
        super().__init__(pevent=pevent, **kargs)
    
    def process_IN_CLOSE_WRITE(self, event: inotify.Event):
        if event.pathname == tsconfig_path:
            logging.info('TSConfig has changed, reloading configuration file')
            update_ts_config()
            return
        logging.info('Source file ' + event.pathname + ' has changed')
        if subprocess.call(['tsc']) == 0:
	        logging.info('Recompiled ' + event.pathname)
        else:
            logging.error('Failed to recompile ' + event.pathname)

    def process_IN_DELETE(self, event: inotify.Event):
        if event.pathname == tsconfig_path:
            logging.error('TSConfig was removed, exiting program')
            exit(1)


update_ts_config()
handler = FileEventHandler()
notifier = inotify.Notifier(watcher_manager, handler)
notifier.loop()
