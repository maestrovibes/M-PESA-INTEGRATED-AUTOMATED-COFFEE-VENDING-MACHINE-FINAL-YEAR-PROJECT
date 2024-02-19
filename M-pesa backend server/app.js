import express from 'express';
import axios from 'axios';
import moment from 'moment';
import cors from 'cors';
import mongoose from 'mongoose';
import Payment from './models/Payment.js'
import 'dotenv/config'
const app = express();

app.use(express.json());
app.use(express.urlencoded({ extended: true}));
//app.use(cors({ origin: process.env.REMOTE_CLIENT_APP, credentials: true }));
app.use(cors());


app.get('/', (req, res) => {
   res.send('Hello World!');
});


app.get('/access_token', (req, res, next) => {
    createToken()
    .then((token) => {
      res.status(200).json({message: "Your access token is " + token});
    })
    .catch(console.log);
});

app.post('/stkpush', (req, res, next) => {
    //const { Amount, PhoneNumber } = req.body; //{} - deconstruct body
    const amount = req.body.amount;
    //const phone = req.body.phone;
    const phone = req.body.phone.substring(1);
    const Amount = amount;
    //const Phone = phone;
    return new Promise((resolve, reject) => {
        createToken()
        .then((token) => {
            const url =
            "https://sandbox.safaricom.co.ke/mpesa/stkpush/v1/processrequest",
            auth = "Bearer " + token;
            var timestamp = moment().format("YYYYMMDDHHmmss");
            const password = new Buffer.from(
            "174379" +
                "bfb279f9aa9bdbcf158e97dd71a467cd2e0c893059b10f78e6b72ada1ed2c919" +
                timestamp
            ).toString("base64");
            
            axios({
                method: 'post',
                url: url,
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': auth
                },
                data: {
                    "BusinessShortCode": 174379,
                    "Password": password,
                    "Timestamp": timestamp,
                    "TransactionType": "CustomerPayBillOnline",
                    "Amount": Amount,
                    "PartyA": `254${phone}`,
                    "PartyB": 174379,
                    "PhoneNumber": `254${phone}`,
                    "CallBackURL": "https://bfdb-197-232-61-213.ngrok.io/callback",
                    "AccountReference": "Premium Coffee",
                    "TransactionDesc": "Payment of Coffee" 
                }
            })
            .then(response => {
                console.log(response.data);
                //res.send(response.data.CustomerMessage);
                res.status(200).json(response.data.CustomerMessage);
                resolve(response.data);
            })
            .catch((err) => {
                console.log(err.message);
            });
        });
    });
});

app.post('/callback', (req, res, next) => {
    //const result = req.body;
    console.log('---callback---')
    console.log(req.body.Body);

    let status = req.body.Body.stkCallback.ResultCode;

    if(status <= 0){
        console.log("Transaction successful!!");

        const phone = req.body.Body.stkCallback.CallbackMetadata.Item[3].Value;
        const amount = req.body.Body.stkCallback.CallbackMetadata.Item[0].Value;
        const transaction_id = req.body.Body.stkCallback.CallbackMetadata.Item[1].Value;
        const resultCode = req.body.Body.stkCallback.ResultCode;

        const payment = new Payment();

        payment.number = phone;
        payment.amount = amount;
        payment.transaction_id = transaction_id;
        payment.resultCode = resultCode;

        payment
        .save()
        .then((data) => {
            console.log({message: "Saved successfully"});
        })
        .catch((err) => {
            console.log(err.message);
        });

        return res.json("Ok");
        
    } else {
        console.log("Transaction failed!!");

        const resultCode = req.body.Body.stkCallback.ResultCode;

        const payment = new Payment();

        payment.resultCode = resultCode;
        payment
        .save()
        .then((data) => {
            console.log({message: "Saved successfully"});
        })
        .catch((err) => {
            console.log(err.message);
        });

        return res.json("Ok");
    }
});

app.get('/confirm', async (req, res, next) => {

    let resultCode;
    try {
        resultCode = await Payment.find({}, {_id: 0, resultCode: 1}).sort({$natural: -1}).limit(1);
    } catch (err) {
        return console.log(err);
    }

    if(!resultCode) {
        return res.status(404).json({message: "No resultCode found!"});
    }
    //let code = resultCode.resultCode;
    return res.status(200).json(resultCode[0].resultCode);
});

function createToken() {
    return new Promise((resolve, reject) => {
        const secret = "lFUaGEIzv0wrmEAc";
        const consumer = "4Fq8CmmlyyhCEbW3pGfmNE2udSCyvlT0";
        const auth = new Buffer.from(`${consumer}:${secret}`).toString("base64");

        axios.get('https://sandbox.safaricom.co.ke/oauth/v1/generate?grant_type=client_credentials', {
            headers: {
                'Authorization': `Basic ${auth}`
            }
        })
        .then(res => {
            console.log(res.data.access_token);
            const token = res.data.access_token;
            resolve(token);
        })
        .catch((err) => {
            console.log(err.message);
        });
    });
}


mongoose
.connect("mongoDBlink"
)
.then(() => app.listen(5000))
.then(() => 
    console.log("Connected to database and listening to port 5000")
)
.catch((err) => console.log(err));
