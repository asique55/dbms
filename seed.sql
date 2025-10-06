
INSERT INTO Rides (ride_id, rider_id, driver_id, created_at, completed_at, route_id,status_id) VALUES
(1, 3, 3, '2025-01-10 10:00:00', '2025-01-10 10:15:00',2,4),
(2, 4, 9, '2025-09-01 09:00:00', '2025-09-01 09:20:00',3,3),
(3, 5, 10, '2023-09-01 09:00:00', '2023-09-01 09:25:00',4,4),
(4, 6, 11, '2018-05-01 09:00:00', '2018-05-01 09:10:00',5,4);


UPDATE rides set completed_at = null WHERE ride_id = 2


--A
SELECT ride_id, created_at, completed_at,
       age(completed_at, created_at) AS ride_duration,
       extract('month' FROM created_at) AS ride_month
FROM Rides
WHERE extract('year' FROM created_at) = extract('year' FROM current_date)
  AND completed_at IS NOT NULL;

UPDATE Rides
SET completed_at = now()
WHERE completed_at IS NULL
  AND created_at < current_timestamp - interval '1 hour';

DELETE FROM Rides
WHERE date_trunc('month', created_at) = date_trunc('month', current_date - interval '2 years')
  OR completed_at < current_date - interval '5 years';

--B
SELECT user_id,
       upper(full_name) AS name_upper,
       substring(phone_number, 1, 3) AS country_code,
       regexp_replace(phone_number, '\D', '', 'g') AS digits_only
FROM Users
WHERE position('7' IN phone_number) > 1
  AND length(full_name) > 3;

UPDATE Users
SET full_name = concat_ws('_', upper(split_part(full_name, ' ', 1)), initcap(split_part(full_name, ' ', 2)))
WHERE position(' ' IN full_name) > 0
  AND lower(full_name) LIKE 'askar%';

DELETE FROM Users
WHERE trim(lower(split_part(full_name, ' ', 1))) = 'aigerim'
   OR regexp_replace(phone_number, '\D', '', 'g') = '';

--C

INSERT INTO Payments (payment_id, ride_id, method, amount, paid_at) VALUES
(1, 1,'Kaspi', 1200.75, now()),
(2, 2,'Cash', 500.49, now()),
(3, 3,'Kaspi', 0, now());

SELECT * FROM Payments

ALTER TABLE Ratings ADD COLUMN created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP

INSERT INTO Ratings (rating_id, ride_id,rating_value, comment, created_at) VALUES
(1, 1, 5, 'Öte jaqsı sapar', now()),
(2, 2, 4, 'Too high', now()),
(3, 3, 1, 'Too low', now());

SELECT payment_id, amount,
       round(amount) AS rounded_amount,
       mod(cast(amount AS int), 5) AS mod5,
       sqrt(amount) AS sqrt_amt
FROM Payments
WHERE amount > 0
  AND floor(amount) <> ceil(amount);

UPDATE Payments
SET amount = ceil(amount)
WHERE abs(amount - round(amount)) > 0.3
  AND amount > 0;

DELETE FROM Ratings
WHERE mod(rating_value, 2) = 0
