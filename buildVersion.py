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

p = Popen(['git', 'describe', '--long'], stdout=PIPE)
gitVersion, err = p.communicate()
if err:
	sys.exit(err)

match = re.match(r'v(?P<major>\d+).(?P<minor>\d+).(?P<patch>\d+)-\d+-(?P<rev>[\w]+)', gitVersion)
if not match:
	sys.exit(2)

data = match.groupdict()

time = datetime.datetime.now()
data['datetime'] = time.strftime('%Y/%m/%d %H:%M:%S')

with open('version_h_template.txt') as r:
	text = r.read()
	with open('version.h', 'w') as v:
		output = text.format(**data)
		v.write(output)

sys.exit(0)