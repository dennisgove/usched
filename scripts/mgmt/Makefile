SYSSBINDIR=`cat ../../.dirsbin`

install:
	if [ -f "`which python`" ]; then make install_python; else make install_sh; fi

install_python:
	cp usched.py ${SYSSBINDIR}/usched

install_sh:
	cp usched.sh ${SYSSBINDIR}/usched

