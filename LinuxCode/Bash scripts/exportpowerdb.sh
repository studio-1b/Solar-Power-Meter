#!/bin/bash

DEST_IP=$1
NC_FLAGS=$2
echo started $(date)
echo sending vanpower database to $DEST_IP
if [ $NC_FLAGS != "" ]; then
    echo netcat flags=$NC_FLAGS
fi

TMP_FILE=$(mktemp --tmpdir)
mysqldump -u dbcreator -p'[user to create temporary database for import/export]' vanpower memory_status instant_read minute_by_minute daily_avg_on_hour ntp_ntps rtc_vs_ntp > $TMP_FILE && cat $TMP_FILE | nc $NC_FLAGS $DEST_IP 10000
rm $TMP_FILE
echo finishd $(date)
