#
# Regular cron jobs for the dl2 package.
#
0 4	* * *	root	[ -x /usr/bin/dl2_maintenance ] && /usr/bin/dl2_maintenance
