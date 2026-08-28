create table tbl_account(
    fld_dbid           integer not null primary key autoincrement,
    fld_account        text    not null,
    fld_password       text    not null
) strict;

insert into tbl_account(fld_dbid, fld_account, fld_password)
values
    (1, 'admin', 'admin');

create table tbl_char(
    fld_dbid           integer not null primary key,
    fld_name           text    not null,
    fld_gender         integer not null check(fld_gender in (0, 1)),
    fld_job            integer not null check(jobValid(fld_job)),
    fld_map            integer not null,
    fld_mapx           integer not null,
    fld_mapy           integer not null,
    fld_hp             integer default 10,
    fld_mp             integer default 10,
    fld_exp            integer default 0,
    fld_gold           integer default 10000,
    fld_hair           integer default 0,
    fld_haircolor      integer default 0,
    fld_pkpoint        integer default 0,

    foreign key (fld_dbid) references tbl_account(fld_dbid) on delete cascade
) strict;

create table tbl_charvarlist(
    fld_dbid           integer not null,
    fld_var            text    not null,
    fld_value          blob        null,

    foreign key (fld_dbid) references tbl_char(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_var)
) strict;

create table tbl_belt(
    fld_dbid           integer not null,
    fld_belt           integer not null,

    fld_itemid         integer not null,
    fld_count          integer not null,

    foreign key (fld_dbid) references tbl_char(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_belt)
) strict;

create table tbl_wear(
    fld_dbid           integer not null,
    fld_wear           integer not null,

    fld_itemid         integer not null,
    fld_count          integer not null,
    fld_duration       integer not null,
    fld_maxduration    integer not null,
    fld_extattrlist    blob    not null,

    foreign key (fld_dbid) references tbl_char(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_wear)
) strict;

create table tbl_inventory(
    fld_dbid           integer not null,
    fld_itemid         integer not null,
    fld_seqid          integer not null,
    fld_count          integer not null,
    fld_duration       integer not null,
    fld_maxduration    integer not null,
    fld_extattrlist    blob    not null,

    foreign key (fld_dbid) references tbl_account(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_itemid, fld_seqid)
) strict;

create table tbl_secureditemlist(
    fld_dbid           integer not null,
    fld_itemid         integer not null,
    fld_seqid          integer not null,
    fld_count          integer not null,
    fld_duration       integer not null,
    fld_maxduration    integer not null,
    fld_extattrlist    blob    not null,

    foreign key (fld_dbid) references tbl_account(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_itemid, fld_seqid)
) strict;

create table tbl_learnedmagiclist(
    fld_dbid           integer not null,
    fld_magicid        integer not null,
    fld_exp            integer default 0,

    foreign key (fld_dbid) references tbl_char(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_magicid)
) strict;

create table tbl_playerconfig(
    fld_dbid           integer not null,
    fld_magickeylist   blob        null default (x''),
    fld_runtimeconfig  blob        null default (x''),

    foreign key (fld_dbid) references tbl_account(fld_dbid) on delete cascade,
    primary key (fld_dbid)
) strict;

create table tbl_chatgroup(
    fld_id             integer not null primary key autoincrement,
    fld_creator        integer not null,
    fld_createtime     integer not null,
    fld_name           text        null default (x''),
    fld_description    blob        null default (x''),
    fld_announcement   blob        null default (x''),

    foreign key (fld_creator) references tbl_char(fld_dbid) on delete cascade
) strict;

create index tbl_chatgroup_creator_index on tbl_chatgroup(fld_creator);

create table tbl_chatgroupmember(
    fld_group          integer not null,
    fld_member         integer not null,
    fld_permission     integer not null,
    fld_jointime       integer not null,

    foreign key (fld_group ) references tbl_chatgroup(fld_id  ) on delete cascade,
    foreign key (fld_member) references tbl_char     (fld_dbid) on delete cascade,
    primary key (fld_group, fld_member)
) strict;

create index tbl_chatgroupmember_member_index on tbl_chatgroupmember(fld_member);

create table tbl_friend(
    fld_dbid           integer not null,
    fld_friend         integer not null,

    foreign key (fld_dbid  ) references tbl_char(fld_dbid) on delete cascade,
    foreign key (fld_friend) references tbl_char(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_friend)
) strict;

create index tbl_friend_friend_index on tbl_friend(fld_friend);

create table tbl_blacklist(
    fld_dbid           integer not null,
    fld_blocked        integer not null,

    foreign key (fld_dbid   ) references tbl_char(fld_dbid) on delete cascade,
    foreign key (fld_blocked) references tbl_char(fld_dbid) on delete cascade,
    primary key (fld_dbid, fld_blocked)
) strict;

create index tbl_blacklist_blocked_index on tbl_blacklist(fld_blocked);

create table tbl_chatmessage(
    fld_id             integer not null primary key autoincrement,
    fld_timestamp      integer not null,
    fld_refer          integer,
    fld_from           integer not null,
    fld_to             integer not null,
    fld_message        blob        null default (x'')
) strict;

create table tbl_delivery(
    fld_record         text    not null primary key,
    fld_dbid           integer not null,
    fld_messageid      integer,
    fld_timestamp      integer not null,
    fld_claimed        integer not null default 0 check(fld_claimed in (0, 1)),
    fld_claimtime      integer,
    fld_payload        blob    not null,

    foreign key (fld_dbid     ) references tbl_char       (fld_dbid) on delete cascade,
    foreign key (fld_messageid) references tbl_chatmessage(fld_id  ) on delete set null
) strict;

create index tbl_delivery_dbid_index      on tbl_delivery(fld_dbid);
create index tbl_delivery_messageid_index on tbl_delivery(fld_messageid);

create table tbl_auctionitem(
    fld_id             integer not null primary key autoincrement,
    fld_seller         integer not null,
    fld_note           text    not null default '',
    fld_itemid         integer not null,
    fld_seqid          integer not null,
    fld_count          integer not null check(fld_count > 0),
    fld_duration       integer not null check(fld_duration >= 0),
    fld_maxduration    integer not null check(fld_maxduration >= 0),
    fld_extattrlist    blob    not null,
    fld_price          integer not null check(fld_price > 0),
    fld_expiretime     integer not null check(fld_expiretime > 0),

    foreign key (fld_seller) references tbl_char(fld_dbid) on delete cascade,
    unique (fld_seller, fld_itemid, fld_seqid)
) strict;

create index tbl_auctionitem_seller_index     on tbl_auctionitem(fld_seller);
create index tbl_auctionitem_expiretime_index on tbl_auctionitem(fld_expiretime);
