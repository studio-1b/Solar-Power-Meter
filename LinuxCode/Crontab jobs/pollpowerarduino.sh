#!/bin/bash

USERPASS_FILE="/home/bob/vanpowerscripts/prod/loginmysql.conf"
WEB_SERVER="192.168.1.200"
HTML_DIR="/var/www/html"

echo $(date) $1
#/usr/sbin/ntpdate -d -q 192.168.1.201

# read free memory and log
if [ $1 == "freememory" ]; then
   TMP_FILE="/run/shm/freememory.html"
   CSV_FILE="/run/shm/freememory.csv"
   JSON_FILE="/var/www/html/freememory.json"
   FINAL_JSON=""
   curl -S http://$WEB_SERVER/404.htm -o $TMP_FILE
   if [ $? -eq 0 ]; then 
      BATCH_ID=$(date +%s)
      cat $TMP_FILE | tr '\r\n' ' ' | sed 's/<!DOCTYPE HTML><html>//g;s/<br>/,/g;s/<br \/>/,/g;s/=/,/g;' | cut -d, -f1,4 > $CSV_FILE
      LOAD_CMD="LOAD DATA LOCAL INFILE '$CSV_FILE' INTO TABLE memory_status_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\R\N' IGNORE 0 LINES (created_date_str,free_memory) set batch_id=$BATCH_ID;"
      echo $LOAD_CMD | mysql --defaults-extra-file=$USERPASS_FILE vanpower
      echo "SELECT * FROM memory_status_import WHERE batch_id=$BATCH_ID and error IS NOT NULL;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower

      # https://stackoverflow.com/questions/16101495/how-can-i-suppress-column-header-output-for-a-single-sql-statement
      echo "SELECT CONCAT('{\"created_date\": \"',created_date,'\", \"created_epoch\": ',UNIX_TIMESTAMP(created_date),', \"free_memory\": ',free_memory,'},') FROM memory_status ORDER BY created_date DESC LIMIT 60;" | mysql --defaults-extra-file=$USERPASS_FILE -N -s vanpower | sed '1i\
[
$a\
null]' > $JSON_FILE

   fi
fi

# read every minute amp and voltage and log
#  if time is within 5min, use remote time.  otherwise local time.
#  log if response is recorded or not.  
#  That way, a non-response is affirmative and will reflect on graph.
if [ $1 == "instant" ]; then
   TMP_FILE="/run/shm/powermeter.html"
   CSV_FILE="/run/shm/powermeter.csv"
   curl -S http://$WEB_SERVER/powermeter -o $TMP_FILE
   if [ $? -eq 0 ]; then
      BATCH_ID=$(date +%s)
      cat $TMP_FILE |  cut -d, -f3 | tr '\r\n' ',' | tr '<br>' ' ' | cut -c21- > $CSV_FILE
      # Thu 12/19/2019 15:47:13 PST(-8)    / ,,Value,11895,-800,-9549,11788,100,300000,124515, /html ,,
      LOAD_CMD="LOAD DATA LOCAL INFILE '$CSV_FILE' INTO TABLE instant_read_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 0 LINES (timestamp_str,@dummy,@dummy,system_mv,battery_ma,@dummy,solar_mv,solar_ma,est_capacity_mah,est_available_mah) set batch_id=$BATCH_ID;"
      echo $LOAD_CMD | mysql --defaults-extra-file=$USERPASS_FILE vanpower
      echo "SELECT * FROM instant_read_import WHERE batch_id=$BATCH_ID AND error IS NOT NULL;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower

      echo "SELECT CONCAT('{\"timestamp\": \"',timestamp, '\",\"battery_ma\": ', ifnull(battery_ma,'null'),', \"system_mv\": ', ifnull(system_mv,'null'),', \"solar_ma\": ',ifnull(solar_ma,'null'),', \"solar_mv\": ', ifnull(solar_mv,'null'),', \"est_capacity_mah\": ',ifnull(est_capacity_mah,'null'),', \"est_available_mah\": ',ifnull(est_available_mah,'null'), '},') FROM instant_read ORDER BY timestamp DESC LIMIT 1440;" | mysql --defaults-extra-file=$USERPASS_FILE -N -s vanpower | sed '1i\
[
$a\
null]' > $HTML_DIR/powermeter.json
   else
      echo "INSERT INTO instant_read (batch_id,timestamp,system_mv,battery_ma,solar_mv,solar_ma) VALUES (1,NOW(),NULL,NULL,NULL,NULL);" | mysql -N -s --defaults-extra-file=$USERPASS_FILE vanpower
   fi
fi

# read every 10min, and extract missing records
#  use remote time.  It should be good enough as a key
#  im not sure there is any accurate method in determining the validity of remote time sent
#  perhaps save the createdate of the record...
if [ $1 == "intervalaverages" ]; then
   TMP_FILE="/run/shm/powerhistory.csv"
   CSV_FILE="/run/shm/powerhistory.csv"
   curl -S http://$WEB_SERVER/powerhistory -o $TMP_FILE

   if [ $? -eq 0 ]; then 
      BATCH_ID=$(date +%s)
      # cat $TMP_FILE |  cut -d, -f3 | tr '\r\n' ',' | tr '<br>' ' ' | cut -c21- > $CSV_FILE
      LOAD_CMD="LOAD DATA LOCAL INFILE '$CSV_FILE' INTO TABLE minute_by_minute_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 2 LINES (timestamp_epoch,battery_ma,system_mv,solar_ma,solar_mv) set batch_id=$BATCH_ID;"
      echo $LOAD_CMD | mysql --defaults-extra-file=$USERPASS_FILE vanpower
      echo "SELECT * FROM minute_by_minute_import WHERE batch_id=$BATCH_ID AND error IS NOT NULL;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower
      echo "SELECT CONCAT('{\"timestamp\": \"',timestamp, '\",\"battery_ma\": ', battery_ma,', \"system_mv\": ', system_mv,', \"solar_ma\": ',solar_ma,', \"solar_mv\": ', solar_mv, '},') FROM minute_by_minute ORDER BY timestamp DESC LIMIT 144 ;" | mysql --defaults-extra-file=$USERPASS_FILE -N -s vanpower | sed '1i\
[
$a\
null]' > $HTML_DIR/latestintervalavgs.json
      echo Data loaded, latest JSON generated
   fi
fi

# read every hour, extract missing records
#  Im not sure what to use as key here...
#  using hour would overwrite previous hours for previous restarts.
#  but I want hour to reset after etiher a restart or overflow...
#  every time it is less than previous, reset...
if [ $1 == "hourlyavgmah" ]; then
   TMP_FILE="/run/shm/hourly.csv"
   CSV_FILE="/run/shm/hourly.csv"
   curl -S http://$WEB_SERVER/hourly -o $TMP_FILE
   if [ $? -eq 0 ]; then 
      # cat $TMP_FILE |  cut -d, -f3 | tr '\r\n' ',' | tr '<br>' ' ' | cut -c21- > $CSV_FILE
      LOAD_CMD="LOAD DATA LOCAL INFILE '$CSV_FILE' INTO TABLE daily_avg_on_hour_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 2 LINES (hour_utc,hour_local,mah_avg,mah_total,sample_count) set batch_id=MONTH(NOW());"
      echo $LOAD_CMD | mysql --defaults-extra-file=$USERPASS_FILE vanpower

      echo "SELECT * FROM daily_avg_on_hour_import WHERE error IS NOT NULL;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower
      echo "SELECT CONCAT('{\"for_month\": null, \"hour_utc\": \"',hour_utc,'\", \"hour_local\": ',hour_local, ', \"mah_avg\": ', cast(sum(mah_total)/sum(sample_count) AS SIGNED),', \"mah_total\": ',sum(mah_total), ', \"sample_count\": ', sum(sample_count), '},') FROM daily_avg_on_hour WHERE mah_total IS NOT NULL AND sample_count IS NOT NULL  GROUP BY hour_utc, hour_local;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower -N -s | sed '1i\
[
$a\
null]' > $HTML_DIR/houravg.old.json

      echo "[" > $HTML_DIR/houravg.json
      echo "SELECT CONCAT('{\"for_month\": null, \"hour_utc\": \"',hour_utc,'\", \"hour_local\": ',hour_local, ', \"mah_avg\": ', cast(sum(mah_total)/sum(sample_count) AS SIGNED),', \"mah_total\": ',sum(mah_total), ', \"sample_count\": ', sum(sample_count), '},') FROM daily_avg_on_hour WHERE mah_total IS NOT NULL AND sample_count IS NOT NULL  GROUP BY hour_utc, hour_local;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower -N -s | sed '1i\
[
$a\
null],' >> $HTML_DIR/houravg.json
      #echo "SELECT CONCAT(IF(b.hr=0,'[',''),'{\"for_month\": ',b.mo,', \"hour_utc\": \"',b.hr,'\", \"hour_local\": ',(b.hr-8+24) MOD 24, ', \"mah_avg\": ', IFNULL(ROUND(sum(a.mah_total)/sum(a.sample_count)),0),', \"mah_total\": ',IFNULL(sum(a.mah_total),0), ', \"sample_count\": ', IFNULL(sum(a.sample_count),0), '},', IF(b.hr=23,'null],',''))     FROM (SELECT m.ID as mo, h.ID as hr FROM month_dim m CROSS JOIN hour_dim h) as b    LEFT JOIN daily_avg_on_hour a ON b.mo = a.for_month AND b.hr=a.hour_utc   GROUP BY b.mo,b.hr   ORDER BY b.mo,b.hr;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower -N -s >> $HTML_DIR/houravg.json
      echo "SELECT CONCAT(IF(b.hr=0,'[',''),'{\"for_month\": ',b.mo,', \"hour_utc\": \"',b.hr,'\", \"hour_local\": ',(b.hr-8+24) MOD 24, ', \"solar_mah_avg\": ', IFNULL(c.mah_avg,0), ', \"mah_avg\": ', IFNULL(a.mah_avg,0),', \"mah_total\": ',IFNULL(a.mah_total,0), ', \"sample_count\": ', IFNULL(a.sample_count,0), '},', IF(b.hr=23,'null],',''))    FROM (SELECT m.ID as mo, h.ID as hr FROM month_dim m CROSS JOIN hour_dim h) as b      LEFT JOIN (SELECT for_month, hour_utc, hour_local, ROUND(sum(mah_total)/sum(sample_count)) mah_avg, SUM(mah_total) mah_total, SUM(sample_count) sample_count FROM daily_avg_on_hour WHERE hour_local IS NOT NULL GROUP BY for_month, hour_utc, hour_local) a ON b.mo = a.for_month AND b.hr=a.hour_utc     LEFT JOIN (SELECT MONTH(timestamp) for_month, (HOUR(timestamp)+8) MOD 24 hour_utc, HOUR(timestamp) hour_local, IFNULL(ROUND(avg(solar_ma)),0) mah_avg,IFNULL(sum(solar_ma),0) mah_total, IFNULL(COUNT(solar_ma),0) sample_count  FROM minute_by_minute GROUP BY MONTH(timestamp), (HOUR(timestamp)+8) MOD 24, HOUR(timestamp)) as c ON b.mo = c.for_month AND b.hr=c.hour_utc     ORDER BY b.mo,b.hr;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower -N -s >> $HTML_DIR/houravg.json
      echo "null]" >> $HTML_DIR/houravg.json

      echo Data loaded, latest JSON generated

      echo "SELECT CONCAT('{\"date\": \"',datestamp,'\", \"battery_mah\": ', battery_mah, ', \"solar_mah\": ', solar_mah, '},' ) FROM (SELECT DATE(timestamp) datestamp, ROUND(24*AVG(battery_ma)) battery_mah, ROUND(24*AVG(solar_ma)) solar_mah FROM minute_by_minute WHERE DATE(timestamp)<>DATE(NOW()) GROUP BY DATE(timestamp) ORDER BY DATE(timestamp) DESC LIMIT 365) as a WHERE datestamp IS NOT NULL AND battery_mah IS NOT NULL AND solar_mah IS NOT NULL;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower -N -s | sed '1i\
[
$a\
null]' > $HTML_DIR/annualamp.json
      echo Data loaded, annual JSON generated
   fi
fi

# NTP should be read every 12 hours
#  each datapoint should have a date associated with it, but it wont....
#  we can estimate a datapoint by doing a running sum in reverse of the last number
#    but what is the last number
# until we can determine a reset, it will be determined by either a change in a reset date
#   that isn't implemented yet
#   or the slot number value has changed
if [ $1 == "ntp" ]; then
   TMP_FILE="/run/shm/ntp.html"
   CSV_FILE="/run/shm/ntp.csv"
   curl -S http://$WEB_SERVER/ntp -o $TMP_FILE
   if [ $? -eq 0 ]; then
      BATCH_ID=$(date +%s)
      head -n 9 $TMP_FILE | cut -d= -f2 | sed "s/<br \/>//g; s/<br>//g; s/<\!DOCTYPE HTML><html>//g" | tr ',' ' ' | tr '\r' ' ' | tr '\n' ','  >> $CSV_FILE

      # BATCH_ID=$(cut -d, -f2 $CSV_FILE)
      echo $BATCH_ID

      # Tue 12/24/2019 5:55:15 PST(-8) , 1577187139  ,Tue 12/24/2019 5:48:55 PST(-8) ,1577195335  ,0.0.0.0 ,0 ,0  ,-3+(rtc elapsed)*(32000/31519)  ,2  ,Tue 12/24/2019 6:09:47 PST(-8) , 1577187139  ,Tue 12/24/2019 6:06:12 PST(-8) ,1577196372  ,0.0.0.0 ,0 ,0  ,-3+(rtc elapsed)*(32000/31476)  ,0  ,
      LOAD_CMD="LOAD DATA LOCAL INFILE '$CSV_FILE' INTO TABLE ntp_ntps_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 0 LINES (timestamp_str, first_sync_epoch, last_ntp_client_date_str, last_ntp_client_response, last_ntps_req_ipaddr, last_ntps_req_response, ntps_response_count, drift_equation_str, current_est_drift) set batch_id=$BATCH_ID;"
      echo $LOAD_CMD | mysql --defaults-extra-file=$USERPASS_FILE vanpower

      LOAD_CMD2="LOAD DATA LOCAL INFILE '$TMP_FILE' INTO TABLE rtc_vs_ntp_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 9 LINES (sample,expected_elapsed_rtc_sec,actual_elapsed_ntp_sec) set batch_id=$BATCH_ID;"
      echo $LOAD_CMD2 | mysql --defaults-extra-file=$USERPASS_FILE vanpower

      echo "SELECT CONCAT('{\"timestamp\": \"', IFNULL(timestamp,NOW()), '\", \"expected_elapsed_rtc_sec\": ', expected_elapsed_rtc_sec, ', \"actual_elapsed_ntp_sec\": ', actual_elapsed_ntp_sec, '},') FROM rtc_vs_ntp ORDER BY id DESC LIMIT 500;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower -N -s | sed '1i\
[
$a\
null]' > $HTML_DIR/ntp.json

   fi
fi

if [ $1 == "calibration" ]; then
    echo "SELECT CONCAT('{\"datestamp\": \"', a.datestamp, '\", \"datacount\": ', a.datacount, ', \"avg_solar_ma\": ', a.avg_solar_ma, ', \"avg_battery_ma\": ', a.avg_battery_ma, ', \"night_solar_ma\": ', a.night_battery_ma, ', \"min_time\": \"', a.min_time, '\", \"max_time\": \"', a.max_time, '\", \"start_mv\": ', b.system_mv, ', \"end_mv\": ', c.system_mv, ', \"diff_mv\": ', c.system_mv - b.system_mv, '},')  FROM (SELECT date(timestamp) datestamp, count(0) datacount, avg(solar_ma) avg_solar_ma, avg(battery_ma) avg_battery_ma, sum(IF(hour(timestamp)>=23 OR hour(timestamp)<=4,battery_ma,0)) night_battery_ma, min(timestamp) min_time, max(timestamp) max_time  FROM minute_by_minute  GROUP BY date(timestamp)   HAVING count(0)>=140 AND COUNT(0)<=144) a  INNER JOIN minute_by_minute b ON a.datestamp = date(b.timestamp) AND a.min_time=b.timestamp  INNER JOIN minute_by_minute c ON a.datestamp = date(c.timestamp) AND a.max_time=c.timestamp WHERE b.system_mv<13000 AND c.system_mv<13000 AND YEAR(a.datestamp)>=2020;" | mysql --defaults-extra-file=$USERPASS_FILE vanpower -N -s | sed '1i\
[
$a\
null]' > $HTML_DIR/ammetercalibration.json
fi


