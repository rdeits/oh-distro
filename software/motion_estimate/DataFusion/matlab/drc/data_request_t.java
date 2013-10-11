/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class data_request_t implements lcm.lcm.LCMEncodable
{
    public byte type;
    public byte period;
 
    public data_request_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x2019583011494b1eL;
 
    public static final byte MINIMAL_ROBOT_STATE = (byte) 0;
    public static final byte AFFORDANCE_LIST = (byte) 1;
    public static final byte CAMERA_IMAGE_HEAD = (byte) 10;
    public static final byte CAMERA_IMAGE_LHAND = (byte) 11;
    public static final byte CAMERA_IMAGE_RHAND = (byte) 12;
    public static final byte CAMERA_IMAGE_LCHEST = (byte) 13;
    public static final byte CAMERA_IMAGE_RCHEST = (byte) 14;
    public static final byte MAP_CATALOG = (byte) 20;
    public static final byte OCTREE_SCENE = (byte) 21;
    public static final byte HEIGHT_MAP_SCENE = (byte) 22;
    public static final byte HEIGHT_MAP_CORRIDOR = (byte) 23;
    public static final byte HEIGHT_MAP_COARSE = (byte) 24;
    public static final byte HEIGHT_MAP_DENSE = (byte) 25;
    public static final byte DEPTH_MAP_SCENE = (byte) 26;
    public static final byte DEPTH_MAP_WORKSPACE = (byte) 27;
    public static final byte DENSE_CLOUD_BOX = (byte) 28;
    public static final byte STEREO_MAP_HEAD = (byte) 30;
    public static final byte STEREO_MAP_LHAND = (byte) 31;
    public static final byte STEREO_MAP_RHAND = (byte) 32;
    public static final byte TERRAIN_COST = (byte) 40;

    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.data_request_t.class))
            return 0L;
 
        classes.add(drc.data_request_t.class);
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
        outs.writeByte(this.type); 
 
        outs.writeByte(this.period); 
 
    }
 
    public data_request_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public data_request_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.data_request_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.data_request_t o = new drc.data_request_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.type = ins.readByte();
 
        this.period = ins.readByte();
 
    }
 
    public drc.data_request_t copy()
    {
        drc.data_request_t outobj = new drc.data_request_t();
        outobj.type = this.type;
 
        outobj.period = this.period;
 
        return outobj;
    }
 
}
