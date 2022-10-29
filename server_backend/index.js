const express = require('express');
const cors = require('cors');
const morgan = require('morgan');
const path = require('path');
const app = express();
require('dotenv').config();

app.use(express.urlencoded({extended: true})); 
app.use(express.json());  
app.use(cors());
app.use(morgan('dev'));
app.use(express.static(path.join(__dirname,'public')))

app.set('PORT', process.env.PORT || 3005);
app.use('/api/',require('./routes/routes'));

app.listen(app.get('PORT'), () => {
    console.log(`Server backend running at http://localhost:${app.get('PORT')}`);
});

