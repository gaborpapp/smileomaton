#!/usr/bin/env python
import os, re

BundleDir = 'Smileomaton.app'

# find all executable files, dynamic libraries, frameworks, whose install
# names have to be changed
def find_executables(bundledir):
	pipe = os.popen('find %s -perm -755 -type f' % bundledir)
	dtargets = pipe.readlines()
	pipe.close()
	dtargets = map(lambda (l): l[:-1], dtargets) # remove lineends
	dtargets = filter(lambda (l): l[-3:] != '.ss', dtargets) # remove executable scripts
	return dtargets

# returns the shared libraries used by target
# first one in the list is the shared library identification name if
# it is a dynamic library or framework
def get_sharedlibs(target):
	file = os.popen('otool -L %s' % target)
	dlibs = file.readlines()
	file.close()
	return dlibs[1:]

install_names = {}
target_dlibs = {}

def change_id(target):
	global install_names
	print 'processing %s...' % target
	dlibs = get_sharedlibs(target)
	dlibs = map(lambda (d): d.strip().split()[0], dlibs)
	# change identification name of shared library or framework
	if ((target.find('framework') > -1) or (target.find('dylib') > -1)) and \
		(dlibs[0].find('@') == -1):
		id = dlibs[0]
		try:
			m = re.search('.*?/(Frameworks/.+)', target)
			nid = '@executable_path/../' + m.group(1)

			print 'changing identification name %s -> %s' % (id, nid)
			install_names[id] = nid
			os.system('install_name_tool -id %s %s' % (nid, target))
		except:
			pass
		dlibs = dlibs[1:] # remove the id
	target_dlibs[target] = dlibs

def change_dlibs(target):
	print 'processing %s...' % target
	dlibs = target_dlibs[target]
	for d in dlibs:
		if d in install_names:
			print 'changing install name %s -> %s' % (d, install_names[d])
			os.system('install_name_tool -change %s %s %s' % \
				(d, install_names[d], target))

def app_postprocess(bundledir):
	dtargets = find_executables(bundledir)
	for t in dtargets:
		change_id(t)
	for t in dtargets:
		change_dlibs(t)

app_postprocess(BundleDir)

