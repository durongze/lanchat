create database qqdb;

create table user(id int(11) not null,
		  userName char(30) not null,
		  passWord char(30) not null,
		  niName char(30) not null,
		  sex char(10) not null,
		  phone char(20) default null,
		  address char(100) default null,
		  primary key (id)
		)engine = InnoDB default charset = utf-8;

delete  from user where userName="durongze";

alter table user drop primary key;

alter table user add primary key(userName);
