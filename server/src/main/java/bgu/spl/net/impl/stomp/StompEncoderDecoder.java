package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import java.util.LinkedList;
import java.util.List;


public class StompEncoderDecoder implements MessageEncoderDecoder<String>  {

    private List<Byte> bytes = new LinkedList<>();

    @Override
    public byte[] encode(String message) {
        return (message + "\u0000").getBytes(java.nio.charset.StandardCharsets.UTF_8);
    }

    @Override
    public String decodeNextByte(byte nextByte) {
        if (nextByte == '\u0000') {
            byte[] byteArray = new byte[bytes.size()];
            for (int i = 0; i < bytes.size(); i++) {
                byteArray[i] = bytes.get(i);
            }
            String ret = new String(byteArray, java.nio.charset.StandardCharsets.UTF_8);
            bytes.clear();
            return ret;
        }
        bytes.add(nextByte);
        return null; 
    }
}

