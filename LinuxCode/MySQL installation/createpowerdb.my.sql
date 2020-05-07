/* initialize database and user */
CREATE DATABASE vanpower;

SHOW DATABASES;

INSERT INTO mysql.user
(user,host,authentication_string,ssl_cipher,x509_issuer,x509_subject)
VALUES
('vanpoweruser','localhost',PASSWORD('[mysqlpassword]'),'','','');

FLUSH PRIVILEGES;

SELECT * FROM mysql.user;

GRANT ALL PRIVILEGES ON vanpower.* TO vanpoweruser@localhost;

FLUSH PRIVILEGES;

SHOW GRANTS FOR 'vanpoweruser'@'localhost';


/* create tables */

USE vanpower;


/* Lookup tables */

CREATE TABLE month_dim (
  ID INT NOT NULL,
  Abbr varchar(3) NOT NULL,
  Name varchar(10) NOT NULL,
  Days int NOT NULL
);
INSERT INTO month_dim 
(ID, Abbr, Name,Days) VALUES 
(1,"Jan","January",31),
(2,"Feb","February",28),
(3,"Mar","March",31),
(4,"Apr","April",30),
(5,"May","May",31),
(6,"Jun","June",30),
(7,"Jul","July",31),
(8,"Aug","August",31),
(9,"Sep","September",30),
(10,"Oct","October",31),
(11,"Nov","November",30),
(12,"Dec","December",31);

CREATE TABLE hour_dim (
  id INT NOT NULL
);
INSERT INTO hour_dim(id)
VALUES (0),
(1),
(2),
(3),
(4),
(5),
(6),
(7),
(8),
(9),
(10),
(11),
(12),
(13),
(14),
(15),
(16),
(17),
(18),
(19),
(20),
(21),
(22),
(23);

/* Actual tables */
/* get last memory status of arduino, to check for health */
CREATE TABLE memory_status (
  id              INT NOT NULL AUTO_INCREMENT,
  batch_id        INT NOT NULL,
  created_date    DATETIME NULL,
  free_memory     INT NULL,
  PRIMARY KEY (id)
);
/* instant read of last 1sec sampling */
CREATE TABLE instant_read (
  id              INT NOT NULL AUTO_INCREMENT,
  batch_id        INT NOT NULL,
  timestamp       DATETIME NULL,
  system_mv       INT NULL,
  battery_ma      INT NULL,
  solar_mv        INT NULL,
  solar_ma        INT NULL,
  est_capacity_mah  INT NULL,
  est_available_mah INT NULL
  PRIMARY KEY (id)
);
/* average of last historical readings, last 24 hours */
CREATE TABLE minute_by_minute (
  id              INT NOT NULL AUTO_INCREMENT,
  batch_id        INT NOT NULL,
  timestamp       DATETIME NULL,
  system_mv       INT NULL,
  battery_ma      INT NULL,
  solar_mv        INT NULL,
  solar_ma        INT NULL,
  PRIMARY KEY (id)
);
/* daily avg of net battery mAh every hour */
CREATE TABLE daily_avg_on_hour (
  id              INT NOT NULL AUTO_INCREMENT,
  batch_id        INT NOT NULL,
  for_month       INT NULL,
  hour_utc        INT NULL,
  hour_local      INT NULL,
  timezone        INT NULL,
  tz_abbr         VARCHAR(10),

  mah_avg         INT NULL,
  mah_total       INT NULL,
  sample_count    INT NULL,

  PRIMARY KEY (id)
);
/*NTP request and NTPS service stats*/
CREATE TABLE ntp_ntps (
  id                        INT NOT NULL AUTO_INCREMENT,
  batch_id                  INT NOT NULL,
  first_sync_epoch          INT NULL,
  timestamp                 DATETIME NULL,
  last_ntp_client_date      DATETIME NULL,
  last_ntp_client_response  int NULL,
  last_ntp_client_datetime  DATETIME NULL,
  last_ntps_req_response    int NULL,
  last_ntps_req_datetime    DATETIME NULL,

  last_ntps_req_ipaddr      VARCHAR(16),
  ntps_response_count       int NULL,
  drift_b                   int NULL,
  drift_m_numerator         int NULL,
  drift_m_denominator       int NULL,
  PRIMARY KEY (id)
);
CREATE TABLE rtc_vs_ntp (
  id                        INT NOT NULL AUTO_INCREMENT,
  ntp_ntps_id               int NULL,
  batch_id                  INT NOT NULL,
  timestamp                 DATETIME NULL,
  sample                    INT NULL,
  expected_elapsed_rtc_sec  INT NULL,
  actual_elapsed_ntp_sec    INT NULL,
  PRIMARY KEY (id)
);




/* tables for mysqlimport, trigger migrated, cleared at end of load */

CREATE TABLE memory_status_import (
  id                  INT NOT NULL AUTO_INCREMENT,
  batch_id            INT NOT NULL,
  created_date_str    VARCHAR(35) NULL,
  free_memory         INT NULL,
  error           VARCHAR(1024),
  PRIMARY KEY (id)
);

DELIMITER //
CREATE TRIGGER memory_status_import_bi
BEFORE INSERT ON memory_status_import
FOR EACH ROW
BEGIN
  DECLARE errmsg VARCHAR(1024);
  DECLARE format VARCHAR(45);
  DECLARE location int;
  DECLARE parsed DATETIME;
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
  BEGIN
    GET DIAGNOSTICS CONDITION 1 errmsg = MESSAGE_TEXT;
    SET NEW.error = CONCAT("Error occurred:", errmsg);
  END;

  SET @@SESSION.sql_mode='ALLOW_INVALID_DATES';
  SET format = "%a %m/%e/%Y %k:%i:%s";
  SET location = LENGTH(NEW.created_date_str);
  SET parsed = STR_TO_DATE(TRIM(SUBSTRING(NEW.created_date_str,1,location)),format);
  WHILE location >0  AND parsed IS NULL DO
    SET location = location - 1;
    SET parsed = STR_TO_DATE(TRIM(SUBSTRING(NEW.created_date_str,1,location)),format);
  END WHILE;
  IF parsed IS NOT NULL THEN
  /*  SET NEW.created_date_str = TRIM(SUBSTRING(NEW.created_date_str,1,location));*/
  ELSE
    SET parsed = NOW(); 
    /* missing date is not an error condition, just set the current one.  Even the wrong date isnt really important, unlike the other data, which is used to graph using date values, this is primarily used as visual diagnostic.  It will only cause a problem when selecting for specific dates, which the dates will fall between the SQL clause.  And easily diagnosed when viewed holistically that the ID values are out of sequence.  If this really every becomes a problem, add the timestamp_str and timestamp column to make the parsed value different from the automatic date.
*/
  END IF;

  IF NEW.error IS NULL THEN
    IF (SELECT NOT EXISTS(SELECT 1 FROM instant_read WHERE timestamp=parsed)) THEN 
      INSERT INTO memory_status (batch_id, created_date, free_memory)
      VALUES(NEW.batch_id, 
            parsed,
            NEW.free_memory);
    END IF;
  END IF;
END; //
DELIMITER ;


/* last sampling of data */
CREATE TABLE instant_read_import (
  id              INT NOT NULL AUTO_INCREMENT,
  batch_id        INT NOT NULL,
  timestamp_str   VARCHAR(35) NULL,
  system_mv       INT NULL,
  battery_ma      INT NULL,
  solar_mv        INT NULL,
  solar_ma        INT NULL,
  est_capacity_mah  INT NULL,
  est_available_mah INT NULL,
  error           VARCHAR(1024),
  PRIMARY KEY (id)
);

/*select STR_TO_DATE(' Sat 12/14/2019 3:11:33 PST(-8)    ',"%a %m/%e/%Y %k:%i:%s");*/

DELIMITER //
CREATE TRIGGER instant_read_import_bi
BEFORE INSERT ON instant_read_import
FOR EACH ROW
BEGIN
  DECLARE errmsg VARCHAR(1024);
  DECLARE format VARCHAR(45);
  DECLARE location int;
  DECLARE parsed DATETIME;
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
  BEGIN
    GET DIAGNOSTICS CONDITION 1 errmsg = MESSAGE_TEXT;
    SET NEW.error = CONCAT("Error occurred:", errmsg);
  END;

  SET @@SESSION.sql_mode='ALLOW_INVALID_DATES';
  SET format = "%a %m/%e/%Y %k:%i:%s";
  IF NEW.error IS NULL THEN
    SET location = LENGTH(NEW.timestamp_str);
    SET parsed = STR_TO_DATE(TRIM(SUBSTRING(NEW.timestamp_str,1,location)),format);
    WHILE location >0  AND parsed IS NULL DO
      SET location = location - 1;
      SET parsed = STR_TO_DATE(TRIM(SUBSTRING(NEW.timestamp_str,1,location)),format);
    END WHILE;
    IF parsed IS NOT NULL THEN
      SET NEW.timestamp_str = TRIM(SUBSTRING(NEW.timestamp_str,1,location));
    END IF;

    IF (SELECT NOT EXISTS(SELECT 1 FROM instant_read WHERE timestamp=parsed)) THEN 
      INSERT INTO instant_read (batch_id, timestamp, system_mv, battery_ma, solar_mv, solar_ma, est_capacity_mah, est_available_mah)
      VALUES(NEW.batch_id, 
            parsed,
            NEW.system_mv, NEW.battery_ma, NEW.solar_mv, NEW.solar_ma, NEW.est_capacity_mah,NEW.est_available_mah);
    END IF;
  END IF;
END; //
DELIMITER ;

/*
LOAD DATA LOCAL INFILE 'powermeter.csv' INTO TABLE instant_read_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 0 LINES (timestamp_str,@dummy,@dummy,system_mv,battery_ma,solar_mv,solar_ma) set batch_id=1;

select * from instant_read_import;
select * from instant_read;
*/




/* averaged minute by minute, and currently 10min */
CREATE TABLE minute_by_minute_import (
  id               INT NOT NULL AUTO_INCREMENT,
  batch_id         INT NOT NULL,
  timestamp_epoch  INT NULL,
  system_mv        INT NULL,
  battery_ma       INT NULL,
  solar_mv         INT NULL,
  solar_ma         INT NULL,
  error            VARCHAR(1024),
  PRIMARY KEY (id)
);

/*
select FROM_UNIXTIME(1576322474);
delete from minute_by_minute;
delete from minute_by_minute_import;
drop trigger minute_by_minute_import_bi;
*/
DELIMITER //
CREATE TRIGGER minute_by_minute_import_bi
BEFORE INSERT ON minute_by_minute_import
FOR EACH ROW
BEGIN
  DECLARE errmsg VARCHAR(1024);
  DECLARE parsed DATETIME;
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
  BEGIN
    GET DIAGNOSTICS CONDITION 1 errmsg = MESSAGE_TEXT;
    SET NEW.error = CONCAT("Error occurred:", errmsg);
  END;

  IF NEW.timestamp_epoch<>0 THEN
    SET parsed = FROM_UNIXTIME(NEW.timestamp_epoch);

    IF NEW.error IS NULL THEN
      IF parsed IS NOT NULL THEN
        IF (SELECT NOT EXISTS(SELECT 1 FROM minute_by_minute WHERE timestamp=parsed)) THEN 
          INSERT INTO minute_by_minute
          (batch_id, timestamp, system_mv, battery_ma, solar_mv, solar_ma)
          VALUES(NEW.batch_id, 
                 parsed, 
  	         NEW.system_mv, NEW.battery_ma, NEW.solar_mv, NEW.solar_ma);
        ELSE
          SET NEW.error = CONCAT("record already exists:", NEW.timestamp_epoch);
        END IF;
      ELSE
        SET NEW.error = CONCAT("no date for:", NEW.timestamp_epoch);
      END IF;
    END IF;
  ELSE
    SET NEW.error = CONCAT("no date provided:", NEW.timestamp_epoch);
  END IF;
END; //
DELIMITER ;

/*
LOAD DATA LOCAL INFILE 'powerhistory.csv' INTO TABLE minute_by_minute_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 2 LINES (timestamp_epoch,battery_ma,system_mv,solar_ma,solar_mv) set batch_id=1;

select * from minute_by_minute_import;
select * from minute_by_minute;
*/


/* average mAh totals for an time of day */
CREATE TABLE daily_avg_on_hour_import (
  id              INT NOT NULL AUTO_INCREMENT,
  batch_id        INT NOT NULL,
  hour_utc        INT NULL,
  hour_local      INT NULL,
  timezone        INT NULL,
  tz_abbr         VARCHAR(10),

  mah_avg         INT NULL,
  mah_total       INT NULL,
  sample_count    INT NULL,

  error           VARCHAR(1024),
  PRIMARY KEY (id)
);
/*
delete from daily_avg_on_hour;
delete  from daily_avg_on_hour_import;
drop trigger daily_avg_on_hour_import_bi;
*/
DELIMITER //
CREATE TRIGGER daily_avg_on_hour_import_bi
BEFORE INSERT ON daily_avg_on_hour_import
FOR EACH ROW
BEGIN
  DECLARE hourid int;
  DECLARE errmsg VARCHAR(1024);
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
  BEGIN
    GET DIAGNOSTICS CONDITION 1 errmsg = MESSAGE_TEXT;
    SET NEW.error = CONCAT("Error occurred:", errmsg);
  END;

  SET NEW.timezone = NEW.hour_local-NEW.hour_utc;
  IF NEW.timezone>12 THEN
    SET NEW.timezone = NEW.timezone-24;
  END IF;
  IF NEW.timezone<-12 THEN
    SET NEW.timezone = NEW.timezone+24;
  END IF;

  /* we will assume everytime the hourly total is less than the prev, 
     that a reset has occurred or a overflow has occured and generate a new record,
     otherwise update the latest record */
  SELECT ID 
  INTO hourid
  FROM  daily_avg_on_hour 
  WHERE ID=(select MAX(ID) from  daily_avg_on_hour where hour_utc=NEW.hour_utc) 
  AND sample_count<NEW.sample_count;
  IF hourid IS NULL THEN 
    INSERT INTO daily_avg_on_hour
    (batch_id, hour_utc, hour_local, for_month, timezone, tz_abbr, mah_avg, mah_total, sample_count)
    VALUES(NEW.batch_id, NEW.hour_utc, NEW.hour_local, month(now()), NEW.timezone, NEW.tz_abbr,
            NEW.mah_avg, NEW.mah_total, NEW.sample_count);
  ELSE
    UPDATE daily_avg_on_hour
    SET mah_avg=(mah_total+NEW.mah_total)/(sample_count+NEW.sample_count),
        mah_total=mah_total+NEW.mah_total, 
        sample_count=sample_count+NEW.sample_count
    WHERE  id=hourid;
  END IF;
END; //
DELIMITER ;


/*
LOAD DATA LOCAL INFILE 'hourlypower.csv' INTO TABLE daily_avg_on_hour_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 2 LINES (hour_utc,hour_local,mah_avg,mah_total,sample_count) set batch_id=MONTH(NOW());

select * from daily_avg_on_hour_import ;
select * from daily_avg_on_hour;
*/



/* tables for ntp client and server performance */
CREATE TABLE ntp_ntps_import (
  id                        INT NOT NULL AUTO_INCREMENT,
  batch_id                  INT NOT NULL,
  first_sync_epoch          INT NULL,
  timestamp_str             VARCHAR(35) NULL,
  last_ntp_client_date_str  VARCHAR(35) NULL,
  last_ntp_client_response  int NULL,
  last_ntps_req_response    int NULL,
  last_ntps_req_ipaddr      VARCHAR(16),
  ntps_response_count       int NULL,
  drift_equation_str        VARCHAR(50) NULL,
  current_est_drift         int NULL,
  error                     VARCHAR(1024),
  PRIMARY KEY (id)
);

DELIMITER //
CREATE TRIGGER ntp_ntps_import_bi
BEFORE INSERT ON ntp_ntps_import
FOR EACH ROW
BEGIN
  DECLARE format VARCHAR(45);
  DECLARE location int;
  DECLARE locationslash int;
  DECLARE locationend int;
  DECLARE parsed1 DATETIME;
  DECLARE parsed2 DATETIME;
  DECLARE b int;
  DECLARE m int;
  DECLARE mdivisor int;
  DECLARE errmsg VARCHAR(1024);
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
  BEGIN
    GET DIAGNOSTICS CONDITION 1 errmsg = MESSAGE_TEXT;
    SET NEW.error = CONCAT("Error occurred:", errmsg);
  END;

  /*check for zero last outgoing NTP sync and get epoch and add ","
   -- OR different "First" NTP sync epoch
   -- any change in either the value in the element, or the epoch value, and re-insert in table
  */ 
  SET @@SESSION.sql_mode='ALLOW_INVALID_DATES';
  SET format = "%a %m/%e/%Y %k:%i:%s";
  
  SET location = LENGTH(NEW.timestamp_str);
  SET parsed1 = STR_TO_DATE(TRIM(SUBSTRING(NEW.timestamp_str,1,location)),format);
  WHILE location >0  AND parsed1 IS NULL DO
    SET location = location - 1;
    SET parsed1 = STR_TO_DATE(TRIM(SUBSTRING(NEW.timestamp_str,1,location)),format);
  END WHILE;
  IF parsed1 IS NOT NULL THEN
    SET NEW.timestamp_str = TRIM(SUBSTRING(NEW.timestamp_str,1,location));
  END IF;

  SET location = LENGTH(NEW.last_ntp_client_date_str);
  SET parsed2 = STR_TO_DATE(TRIM(SUBSTRING(NEW.last_ntp_client_date_str,1,location)),format);
  WHILE location >0  AND parsed2 IS NULL DO
    SET location = location - 1;
    SET parsed2 = STR_TO_DATE(TRIM(SUBSTRING(NEW.last_ntp_client_date_str,1,location)),format);
  END WHILE;
  IF parsed2 IS NOT NULL THEN
    SET NEW.last_ntp_client_date_str = TRIM(SUBSTRING(NEW.last_ntp_client_date_str,1,location));
  END IF;

  /*2261+(rtc elapsed)*(233/32000)*/
  SET location = LOCATE("+", NEW.drift_equation_str);
  IF location<>0 THEN
    SET b = CONVERT(TRIM(SUBSTRING(NEW.last_ntp_client_date_str,1,location)),UNSIGNED INTEGER);
    SET location = location+16;
    SET locationslash = LOCATE("/", NEW.drift_equation_str);
    SET locationend = LOCATE(")", NEW.drift_equation_str);
    IF locationslash<>0 AND locationend<>0 THEN
      SET m = CONVERT(TRIM(SUBSTRING(NEW.last_ntp_client_date_str,location,locationslash-location)),UNSIGNED INTEGER);
      SET mdivisor = CONVERT(TRIM(SUBSTRING(NEW.last_ntp_client_date_str,locationslash+1,locationend-locationslash)),UNSIGNED INTEGER);
    END IF;
  END IF;


  IF parsed1 IS NULL THEN
    SET NEW.error = CONCAT("unable to parse:",NEW.timestamp_str);
  ELSEIF parsed2 IS NULL THEN
    SET NEW.error = CONCAT("unable to parse:",NEW.last_ntp_client_date_str);
  ELSEIF NEW.error IS NULL THEN
    INSERT INTO ntp_ntps (batch_id, timestamp,last_ntp_client_date,last_ntp_client_response,
                      last_ntp_client_datetime,last_ntps_req_response,last_ntps_req_datetime,
                      last_ntps_req_ipaddr,ntps_response_count,
	              drift_b, drift_m_numerator, drift_m_denominator)
    VALUES(NEW.batch_id, 
          parsed1, 
          parsed2, 
	  NEW.last_ntp_client_response, 
          FROM_UNIXTIME(NEW.last_ntp_client_response),
	  NEW.last_ntps_req_response, 
          FROM_UNIXTIME(NEW.last_ntps_req_response),
	  NEW.last_ntps_req_ipaddr,
	  NEW.ntps_response_count,
	  b, m, mdividsor);
  
  END IF;
END; //
DELIMITER ;



CREATE TABLE rtc_vs_ntp_import (
  id                        INT NOT NULL AUTO_INCREMENT,
  ntp_ntps_import_id        int NULL,
  batch_id                  INT NOT NULL,
  timestamp_str             VARCHAR(35) NULL,
  sample                    INT NULL,
  expected_elapsed_rtc_sec  INT NULL,
  actual_elapsed_ntp_sec    INT NULL,
  error                     VARCHAR(1024),
  PRIMARY KEY (id)
);

DELIMITER //
CREATE TRIGGER rtc_vs_ntp_import_bi
BEFORE INSERT ON rtc_vs_ntp_import
FOR EACH ROW
BEGIN
  DECLARE format VARCHAR(45);
  DECLARE first_sync_epoch int;
  DECLARE import_parent_id int;
  DECLARE old_parent_id int;
  DECLARE old_sync_epoch int;
  DECLARE old_expected_elapsed_rtc_sec int;
  DECLARE old_actual_elapsed_ntp_sec int;
  DECLARE new_parent_id int;
  DECLARE new_sync_epoch int;
  DECLARE existing_id int;
  DECLARE errmsg VARCHAR(1024);
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
  BEGIN
    GET DIAGNOSTICS CONDITION 1 errmsg = MESSAGE_TEXT;
    SET NEW.error = CONCAT("Error occurred:", errmsg);
  END;

  SELECT id, first_sync_epoch
  INTO import_parent_id, first_sync_epoch
  FROM ntp_ntps_import
  WHERE batch_id=NEW.batch_id;

  /* link the import table to the parent... not really important */
  SET NEW.ntp_ntps_import_id = import_parent_id;

  SELECT id, first_sync_epoch
  INTO new_parent_id, new_sync_epoch
  FROM ntp_ntps
  WHERE batch_id=NEW.batch_id;

  /*check for zero last outgoing NTP sync and get epoch and add ","
  -- OR different "First" NTP sync epoch
  -- any change in either the value in the element, or the epoch value, and re-insert into table
  */ 
  IF NEW.error IS NULL THEN
    SELECT id, ntp_ntps_id, first_sync_epoch
         , expected_elapsed_rtc_sec
         , actual_elapsed_ntp_sec
    INTO existing_id, old_parent_id, old_sync_epoch
       , old_expected_elapsed_rtc_sec
       , old_actual_elapsed_ntp_sec
    FROM rtc_vs_ntp
    WHERE ID=(SELECT MAX(id) FROM rtc_vs_ntp WHERE sample=NEW.sample);
 
    IF old_sync_epoch<>new_sync_epoch 
       OR old_expected_elapsed_rtc_sec <> NEW.expected_elapsed_rtc_sec 
       OR old_actual_elapsed_ntp_sec <> NEW.actual_elapsed_ntp_sec THEN
      SET existing_id = NULL;
    END IF;

    IF existing_id IS NULL AND NEW.expected_elapsed_rtc_sec<>0 
                           AND NEW.actual_elapsed_ntp_sec<>0 THEN
      INSERT INTO rtc_vs_ntp 
      (timestamp,ntp_ntps_id,batch_id,sample,expected_elapsed_rtc_sec,actual_elapsed_ntp_sec)
      VALUES
      (now(),new_parent_id,NEW.batch_id,NEW.sample,
       NEW.expected_elapsed_rtc_sec,NEW.actual_elapsed_ntp_sec);
    END IF;
  END IF;
END; //
DELIMITER ;

/*
Sat 12/14/2019 16:54:54 PST(-8) ,
Sat 12/14/2019 15:41:48 PST(-8) ,
1576366908  ,0.0.0.0 ,0 ,0  ,2261+(rtc elapsed)*(233/32000)  ,-2094  

the regression data needs to be loaded first...which means the foreign key needs to be established in advance and not autogenerated... b/c the trigger is on the foreign key table, to move the records over...

LOAD DATA LOCAL INFILE 'ntps.csv' INTO TABLE ntp_ntps_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 0 LINES (timestamp_str,last_ntp_client_date_str,last_ntp_client_response,
last_ntps_req_response
,last_ntps_req_ipaddr,ntps_response_count,drift_equation_str,current_est_drift) set batch_id=4;


LOAD DATA LOCAL INFILE 'rtcntp.csv' INTO TABLE ntp_ntps_import FIELDS TERMINATED BY ',' LINES TERMINATED BY '\r\n' IGNORE 2 LINES (hour_utc,hour_local,mah_avg,mah_total,sample_count) set batch_id=MONTH(NOW());

select * from daily_avg_on_hour_import ;
select * from daily_avg_on_hour;
*/


