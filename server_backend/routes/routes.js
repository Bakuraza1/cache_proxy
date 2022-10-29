const express = require('express');
const router = express.Router();
const fs = require('fs');
const { v4: uuidv4 } = require('uuid');
const path = require('path');

router.get('/', (req, res) => {
	res.send('<h1>Welcome to Server Backend</h1>');
});

router.get('/users', (req, res) => {
	const users = JSON.parse(fs.readFileSync(__dirname + '/../data/users.json'));
	res.send(users);
});

router.post('/users', (req, res) => {
	const { name, email, password, age } = req.body;
	const users = JSON.parse(fs.readFileSync(__dirname + '/../data/users.json'));
	users.push({ id: uuidv4(), name, email, password, age })
	fs.writeFileSync(__dirname + '/../data/users.json', JSON.stringify(users));
	res.send(users[users.length - 1]);
});

router.patch('/users/:id', (req, res) => {
	const { id } = req.params;
	const users = JSON.parse(fs.readFileSync(__dirname + '/../data/users.json'));
	let index = -1;
	let user;
	users.forEach((usr, i) => {
		if (usr.id === id) {
			index = i;
			user = usr;
		}
	});
	if (user) {
		users[index] = { ...users[index], ...req.body };
		fs.writeFileSync(__dirname + '/../data/users.json', JSON.stringify(users));
		res.send(users[index]);
	} else {
		res.send(`User doesn't exists`).status(419);
	}
});

router.delete('/users/:id', (req, res) => {
	const { id } = req.params;
	const users = JSON.parse(fs.readFileSync(__dirname + '/../data/users.json'));
	let index = -1;
	let user;
	users.forEach((usr, i) => {
		if (usr.id === id) {
			index = i;
			user = usr;
		}
	});
	if (user) {
		users.splice(index, 1);
		fs.writeFileSync(__dirname + '/../data/users.json', JSON.stringify(users));
		res.send('User deleted')
	} else {
		res.send(`User doesn't exists`).status(419);
	}

});

router.get('/image', (req, res) => {
	res.sendFile(path.join(__dirname,"../public/Octocat.png"))
});
module.exports = router;
