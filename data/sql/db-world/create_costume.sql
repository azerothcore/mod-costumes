CREATE TABLE IF NOT EXISTS `costume` (
    `item_entry` mediumint UNSIGNED PRIMARY KEY NOT NULL,
    `display_id` mediumint UNSIGNED NOT NULL,
    `scale` float NOT NULL DEFAULT 1,
    `duration` mediumint NOT NULL DEFAULT -1
);

ALTER TABLE `costume` ADD UNIQUE (`item_entry`);
