################################################################################
# smileomaton top level scons script

DEBUG = 0
PROFILE = 0
STATIC = 0

env = Environment()

platform = env['PLATFORM']

BuildDir('build', 'src', duplicate = 0)

SConscript('build/SConscript',
	exports = ['env', 'platform', 'DEBUG', 'PROFILE', 'STATIC'])

