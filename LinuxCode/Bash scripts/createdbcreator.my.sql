CREATE USER 'dbcreator'@'localhost' IDENTIFIED BY '[user to create temporary database for import/export]';

GRANT ALL PRIVILEGES ON *.* TO 'dbcreator'@'localhost';
GRANT GRANT OPTION ON *.* TO 'dbcreator'@'localhost';
