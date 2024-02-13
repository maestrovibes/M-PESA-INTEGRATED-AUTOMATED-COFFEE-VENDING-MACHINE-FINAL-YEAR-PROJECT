import mongoose from 'mongoose';

const Schema = mongoose.Schema;

const paymentSchema = new Schema({
    number: {
        type: String,
    },
    amount: {
        type: String,
    },
    transaction_id: {
        type: String,
    },
    resultCode: {
        type: Number,
        required: true
    },
    paidAt: {
        type: Date,
        default: Date.now
      }
})

//export schema to mongoDB
export default mongoose.model("Payment", paymentSchema);