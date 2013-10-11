/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class driving_controller_status_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public byte status;
    public long time_to_drive;
 
    public driving_controller_status_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x7f27046e8be67fc2L;
 
    public static final byte IDLE = (byte) 0;
    public static final byte DRIVING_ROAD_ONLY_CARROT = (byte) 1;
    public static final byte DRIVING_ROAD_ONLY_ARC = (byte) 2;
    public static final byte DRIVING_TLD_AND_ROAD = (byte) 3;
    public static final byte DRIVING_TLD = (byte) 4;
    public static final byte DRIVING_USER = (byte) 5;
    public static final byte ERROR_NO_MAP = (byte) 6;
    public static final byte ERROR_MAP_TIMEOUT = (byte) 7;
    public static final byte ERROR_TLD_TIMEOUT = (byte) 8;
    public static final byte ERROR_NO_VALID_GOAL = (byte) 9;
    public static final byte DRIVING_DOING_INITIAL_TURN = (byte) 10;
    public static final byte DRIVING_DOING_BRAKING = (byte) 11;

    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.driving_controller_status_t.class))
            return 0L;
 
        classes.add(drc.driving_controller_status_t.class);
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
        outs.writeLong(this.utime); 
 
        outs.writeByte(this.status); 
 
        outs.writeLong(this.time_to_drive); 
 
    }
 
    public driving_controller_status_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public driving_controller_status_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.driving_controller_status_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.driving_controller_status_t o = new drc.driving_controller_status_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.utime = ins.readLong();
 
        this.status = ins.readByte();
 
        this.time_to_drive = ins.readLong();
 
    }
 
    public drc.driving_controller_status_t copy()
    {
        drc.driving_controller_status_t outobj = new drc.driving_controller_status_t();
        outobj.utime = this.utime;
 
        outobj.status = this.status;
 
        outobj.time_to_drive = this.time_to_drive;
 
        return outobj;
    }
 
}
