#1
SELECT name
FROM Pokemon
WHERE type = 'Grass'
ORDER BY name;

#2
SELECT name
FROM Trainer
WHERE hometown = 'Brown City' OR hometown = 'Rainbow City'
ORDER BY name;

#3
SELECT DISTINCT type
FROM Pokemon
ORDER BY type;

#4
SELECT name
FROM City
WHERE name LIKE 'B%'
ORDER BY name;

#5
SELECT DISTINCT hometown
FROM Trainer
WHERE NOT name LIKE 'M%'
ORDER BY hometown;

#6
SELECT nickname
FROM CatchedPokemon
WHERE level = (SELECT MAX(level) FROM CatchedPokemon)
ORDER BY nickname;

#7
SELECT name
FROM Pokemon
WHERE name LIKE 'A%' OR name LIKE 'E%' OR name LIKE 'I%' OR name LIKE 'O%' OR name LIKE 'U%'
ORDER BY name;

#8
SELECT AVG(level)
FROM CatchedPokemon;

#9
SELECT MAX(level)
FROM CatchedPokemon
WHERE owner_id = (SELECT id FROM Trainer WHERE name = 'Yellow');

#10
SELECT DISTINCT hometown
FROM Trainer
ORDER BY hometown;

#11
SELECT T.name, CP.nickname
FROM Trainer T, CatchedPokemon CP
WHERE CP.nickname LIKE 'A%'
ORDER BY T.name;

#12
SELECT name
FROM Trainer
WHERE id = (SELECT leader_id FROM Gym WHERE city = (SELECT name FROM City WHERE description = 'Amazon'));

#13???
SELECT T.id, COUNT(type) AS cnt
FROM (SELECT T.id, COUNT(type) AS cnt
    FROM Trainer AS T
    JOIN CatchedPokemon ON owner_id = T.id 
    JOIN Pokemon ON CatchedPokemon.pid = Pokemon.id AND type = 'Normal') AS SQ
WHERE SQ.cnt = SELECT MAX(SQ.cnt) FROM SQ
GROUP BY T.id;

#14
SELECT DISTINCT type
FROM Pokemon
WHERE id < 10
GROUP BY id DESC;

#15
SELECT COUNT(id)
FROM Pokemon
WHERE NOT type = 'Fire';

#16
SELECT P.name
FROM Pokemon P, Evolution E
WHERE E.before_id > E.after_id AND P.id = E.before_id
ORDER BY name;

#17???
SELECT AVG(CP.level)
FROM Pokemon P, CatchedPokemon CP
WHERE CP.pid = P.id AND P.type = 'Water';

#18
SELECT cp.nickname
FROM CatchedPokemon CP, Gym G
WHERE CP.level = (SELECT MAX(CP.level) FROM CatchedPokemon) AND CP.owner_id = G.leader_id
ORDER BY nickname;

#19

#20

#21???
SELECT name
FROM Trainer
JOIN CatchedPokemon 
ON Trainer.id = owner_id
WHERE Trainer.id = ANY(SELECT leader_id FROM Gym)
#GROUP BY name
ORDER BY name;

#22???
SELECT T.hometown
FROM Trainer AS T, (SELECT COUNT(hometown) AS cnt, FROM Trainer, ORDER BY hometown) AS SQ
WHERE SQ.cnt = (SELECT MAX(SQ.cnt) FROM (SELECT COUNT(hometown) AS cnt, FROM Trainer, ORDER BY hometown))
ORDER BY T.hometown;

#23???
SELECT DISTINCT P.name
FROM CatchedPokemon AS CP, Pokemon AS P
WHERE CP.pid = P.id
  AND CP.pid IN(
    SELECT CP.pid
    FROM CatchedPokemon AS CP, Trainer AS T
    WHERE CP.owner_id = T.id AND T.hometown = 'Sangnok City'
  ) AS SQ1
  AND CP.pid IN(
    SELECT CP.pid
    FROM CatchedPokemon AS CP, Trainer AS T
    WHERE CP.owner_id = T.id AND T.hometown = 'Brown City'
  ) AS SQ2
ORDER BY P.name;

#24???
SELECT T.name
FROM CatchedPokemon AS CP, Trainer AS T
WHERE CP.pid = (SELECT id FROM Pokemon WHERE name = 'P%') AND CP.owner_id = (SELECT id FROM Trainer WHERE hometown = 'Sangnok City')
ORDER BY T.name;

#25
SELECT T.name, nickname
FROM Trainer AS T
JOIN CatchedPokemon ON owner_id = T.id
ORDER BY T.name, nickname;

#26

#27 출력없음
SELECT CP.nickname
FROM CatchedPokemon AS CP, Gym AS G
WHERE CP.owner_id = (SELECT leader_id FROM GYM WHERE city = 'Sangnok') AND CP.pid = (SELECT id FROM Pokemon WHERE type = 'Water')
ORDER BY CP.nickname;

#28 출력없음
SELECT T.name
FROM Trainer AS T
WHERE 3 >= ANY (SELECT CP.pid FROM CatchedPokemon AS CP, Evolution AS E WHERE CP.owner_id = T.id AND CP.pid = E.before_id)
ORDER BY T.name

#29
SELECT DISTINCT P.name
FROM Pokemon AS P, CatchedPokemon AS CP
WHERE NOT P.id = CP.pid
ORDER BY P.name;

#30
SELECT MAX(CP.level) AS level
FROM Trainer AS T
JOIN CatchedPokemon AS CP ON CP.owner_id = T.id
GROUP BY T.hometown
ORDER BY MAX(CP.level) DESC;

#31
