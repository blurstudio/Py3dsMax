##
#   :namespace  buildVersion
#
#   :remarks    Queries git for version specific info and automaticly generates version.h
#   
#   :author     mikeh@blur.com
#   :author     Blur Studio
#   :date       05/11/15
#

import sys
import re
import datetime
from subprocess import Popen, PIPE

def interrogateGit(arguments, regex):
	p = Popen(arguments, stdout=PIPE)
	ret, err = p.communicate()
	if err:
		sys.exit(err)
	match = re.match(regex, ret)
	if not match:
		print 'Unable to find match'
		print [ret]
		print regex
		sys.exit(2)
	return match
	
gitVersion = interrogateGit(
	['git', 'describe', '--long'], 
	r'v(?P<major>\d+).(?P<minor>\d+).(?P<patch>\d+)-\d+-(?P<rev>[\w]+)'
)

data = gitVersion.groupdict()

revCount = interrogateGit(['git', 'rev-list', 'HEAD', '--count'], r'\d+')
data['revCount'] = revCount.group(0)

time = datetime.datetime.now()
data['datetime'] = time.strftime('%Y/%m/%d %H:%M:%S')

with open('version_h_template.txt') as r:
	text = r.read()
	with open('version.h', 'w') as v:
		output = text.format(**data)
		v.write(output)

sys.exit(0)