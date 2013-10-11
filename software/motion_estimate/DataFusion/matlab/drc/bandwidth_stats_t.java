/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class bandwidth_stats_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public long previous_utime;
    public long sim_utime;
    public int num_sent_channels;
    public String sent_channels[];
    public int sent_bytes[];
    public int queued_msgs[];
    public int queued_bytes[];
    public int num_received_channels;
    public String received_channels[];
    public int received_bytes[];
 
    public bandwidth_stats_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x078f90541c2dbdf9L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.bandwidth_stats_t.class))
            return 0L;
 
        classes.add(drc.bandwidth_stats_t.class);
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
        char[] __strbuf = null;
        outs.writeLong(this.utime); 
 
        outs.writeLong(this.previous_utime); 
 
        outs.writeLong(this.sim_utime); 
 
        outs.writeInt(this.num_sent_channels); 
 
        for (int a = 0; a < this.num_sent_channels; a++) {
            __strbuf = new char[this.sent_channels[a].length()]; this.sent_channels[a].getChars(0, this.sent_channels[a].length(), __strbuf, 0); outs.writeInt(__strbuf.length+1); for (int _i = 0; _i < __strbuf.length; _i++) outs.write(__strbuf[_i]); outs.writeByte(0); 
        }
 
        for (int a = 0; a < this.num_sent_channels; a++) {
            outs.writeInt(this.sent_bytes[a]); 
        }
 
        for (int a = 0; a < this.num_sent_channels; a++) {
            outs.writeInt(this.queued_msgs[a]); 
        }
 
        for (int a = 0; a < this.num_sent_channels; a++) {
            outs.writeInt(this.queued_bytes[a]); 
        }
 
        outs.writeInt(this.num_received_channels); 
 
        for (int a = 0; a < this.num_received_channels; a++) {
            __strbuf = new char[this.received_channels[a].length()]; this.received_channels[a].getChars(0, this.received_channels[a].length(), __strbuf, 0); outs.writeInt(__strbuf.length+1); for (int _i = 0; _i < __strbuf.length; _i++) outs.write(__strbuf[_i]); outs.writeByte(0); 
        }
 
        for (int a = 0; a < this.num_received_channels; a++) {
            outs.writeInt(this.received_bytes[a]); 
        }
 
    }
 
    public bandwidth_stats_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public bandwidth_stats_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.bandwidth_stats_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.bandwidth_stats_t o = new drc.bandwidth_stats_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        char[] __strbuf = null;
        this.utime = ins.readLong();
 
        this.previous_utime = ins.readLong();
 
        this.sim_utime = ins.readLong();
 
        this.num_sent_channels = ins.readInt();
 
        this.sent_channels = new String[(int) num_sent_channels];
        for (int a = 0; a < this.num_sent_channels; a++) {
            __strbuf = new char[ins.readInt()-1]; for (int _i = 0; _i < __strbuf.length; _i++) __strbuf[_i] = (char) (ins.readByte()&0xff); ins.readByte(); this.sent_channels[a] = new String(__strbuf);
        }
 
        this.sent_bytes = new int[(int) num_sent_channels];
        for (int a = 0; a < this.num_sent_channels; a++) {
            this.sent_bytes[a] = ins.readInt();
        }
 
        this.queued_msgs = new int[(int) num_sent_channels];
        for (int a = 0; a < this.num_sent_channels; a++) {
            this.queued_msgs[a] = ins.readInt();
        }
 
        this.queued_bytes = new int[(int) num_sent_channels];
        for (int a = 0; a < this.num_sent_channels; a++) {
            this.queued_bytes[a] = ins.readInt();
        }
 
        this.num_received_channels = ins.readInt();
 
        this.received_channels = new String[(int) num_received_channels];
        for (int a = 0; a < this.num_received_channels; a++) {
            __strbuf = new char[ins.readInt()-1]; for (int _i = 0; _i < __strbuf.length; _i++) __strbuf[_i] = (char) (ins.readByte()&0xff); ins.readByte(); this.received_channels[a] = new String(__strbuf);
        }
 
        this.received_bytes = new int[(int) num_received_channels];
        for (int a = 0; a < this.num_received_channels; a++) {
            this.received_bytes[a] = ins.readInt();
        }
 
    }
 
    public drc.bandwidth_stats_t copy()
    {
        drc.bandwidth_stats_t outobj = new drc.bandwidth_stats_t();
        outobj.utime = this.utime;
 
        outobj.previous_utime = this.previous_utime;
 
        outobj.sim_utime = this.sim_utime;
 
        outobj.num_sent_channels = this.num_sent_channels;
 
        outobj.sent_channels = new String[(int) num_sent_channels];
        if (this.num_sent_channels > 0)
            System.arraycopy(this.sent_channels, 0, outobj.sent_channels, 0, this.num_sent_channels); 
        outobj.sent_bytes = new int[(int) num_sent_channels];
        if (this.num_sent_channels > 0)
            System.arraycopy(this.sent_bytes, 0, outobj.sent_bytes, 0, this.num_sent_channels); 
        outobj.queued_msgs = new int[(int) num_sent_channels];
        if (this.num_sent_channels > 0)
            System.arraycopy(this.queued_msgs, 0, outobj.queued_msgs, 0, this.num_sent_channels); 
        outobj.queued_bytes = new int[(int) num_sent_channels];
        if (this.num_sent_channels > 0)
            System.arraycopy(this.queued_bytes, 0, outobj.queued_bytes, 0, this.num_sent_channels); 
        outobj.num_received_channels = this.num_received_channels;
 
        outobj.received_channels = new String[(int) num_received_channels];
        if (this.num_received_channels > 0)
            System.arraycopy(this.received_channels, 0, outobj.received_channels, 0, this.num_received_channels); 
        outobj.received_bytes = new int[(int) num_received_channels];
        if (this.num_received_channels > 0)
            System.arraycopy(this.received_bytes, 0, outobj.received_bytes, 0, this.num_received_channels); 
        return outobj;
    }
 
}
