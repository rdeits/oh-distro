/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class shaper_msg_t implements lcm.lcm.LCMEncodable
{
    public byte channel;
    public byte priority;
    public int message_number;
    public int fragment;
    public byte is_last_fragment;
    public int data_size;
    public byte data[];
 
    public shaper_msg_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x5ebfbbe356db5d0bL;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.shaper_msg_t.class))
            return 0L;
 
        classes.add(drc.shaper_msg_t.class);
        long hash = LCM_FINGERPRINT_BASE
            ;
        classes.remove(classes.size() - 1);
        return (hash<<1) + ((hash>>63)&1);
    }
 
    public void encode(DataOutput outs) throws IOException
    {
        outs.writeLong(LCM_FINGERPRINT);
        _encodeRecursive(outs);
    }
 
    public void _encodeRecursive(DataOutput outs) throws IOException
    {
        outs.writeByte(this.channel); 
 
        outs.writeByte(this.priority); 
 
        outs.writeInt(this.message_number); 
 
        outs.writeInt(this.fragment); 
 
        outs.writeByte(this.is_last_fragment); 
 
        outs.writeInt(this.data_size); 
 
        if (this.data_size > 0)
            outs.write(this.data, 0, data_size);
 
    }
 
    public shaper_msg_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public shaper_msg_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.shaper_msg_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.shaper_msg_t o = new drc.shaper_msg_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.channel = ins.readByte();
 
        this.priority = ins.readByte();
 
        this.message_number = ins.readInt();
 
        this.fragment = ins.readInt();
 
        this.is_last_fragment = ins.readByte();
 
        this.data_size = ins.readInt();
 
        this.data = new byte[(int) data_size];
        ins.readFully(this.data, 0, data_size); 
    }
 
    public drc.shaper_msg_t copy()
    {
        drc.shaper_msg_t outobj = new drc.shaper_msg_t();
        outobj.channel = this.channel;
 
        outobj.priority = this.priority;
 
        outobj.message_number = this.message_number;
 
        outobj.fragment = this.fragment;
 
        outobj.is_last_fragment = this.is_last_fragment;
 
        outobj.data_size = this.data_size;
 
        outobj.data = new byte[(int) data_size];
        if (this.data_size > 0)
            System.arraycopy(this.data, 0, outobj.data, 0, this.data_size); 
        return outobj;
    }
 
}
