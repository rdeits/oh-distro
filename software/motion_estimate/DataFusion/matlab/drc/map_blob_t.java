/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class map_blob_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public byte num_dims;
    public int dimensions[];
    public int stride_bytes[];
    public byte compression;
    public byte data_type;
    public int num_bytes;
    public byte data[];
 
    public map_blob_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x763604aafcc31981L;
 
    public static final byte UNCOMPRESSED = (byte) 0;
    public static final byte ZLIB = (byte) 1;
    public static final byte UINT8 = (byte) 0;
    public static final byte UINT16 = (byte) 1;
    public static final byte FLOAT32 = (byte) 2;

    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.map_blob_t.class))
            return 0L;
 
        classes.add(drc.map_blob_t.class);
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
 
        outs.writeByte(this.num_dims); 
 
        for (int a = 0; a < this.num_dims; a++) {
            outs.writeInt(this.dimensions[a]); 
        }
 
        for (int a = 0; a < this.num_dims; a++) {
            outs.writeInt(this.stride_bytes[a]); 
        }
 
        outs.writeByte(this.compression); 
 
        outs.writeByte(this.data_type); 
 
        outs.writeInt(this.num_bytes); 
 
        if (this.num_bytes > 0)
            outs.write(this.data, 0, num_bytes);
 
    }
 
    public map_blob_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public map_blob_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.map_blob_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.map_blob_t o = new drc.map_blob_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.utime = ins.readLong();
 
        this.num_dims = ins.readByte();
 
        this.dimensions = new int[(int) num_dims];
        for (int a = 0; a < this.num_dims; a++) {
            this.dimensions[a] = ins.readInt();
        }
 
        this.stride_bytes = new int[(int) num_dims];
        for (int a = 0; a < this.num_dims; a++) {
            this.stride_bytes[a] = ins.readInt();
        }
 
        this.compression = ins.readByte();
 
        this.data_type = ins.readByte();
 
        this.num_bytes = ins.readInt();
 
        this.data = new byte[(int) num_bytes];
        ins.readFully(this.data, 0, num_bytes); 
    }
 
    public drc.map_blob_t copy()
    {
        drc.map_blob_t outobj = new drc.map_blob_t();
        outobj.utime = this.utime;
 
        outobj.num_dims = this.num_dims;
 
        outobj.dimensions = new int[(int) num_dims];
        if (this.num_dims > 0)
            System.arraycopy(this.dimensions, 0, outobj.dimensions, 0, this.num_dims); 
        outobj.stride_bytes = new int[(int) num_dims];
        if (this.num_dims > 0)
            System.arraycopy(this.stride_bytes, 0, outobj.stride_bytes, 0, this.num_dims); 
        outobj.compression = this.compression;
 
        outobj.data_type = this.data_type;
 
        outobj.num_bytes = this.num_bytes;
 
        outobj.data = new byte[(int) num_bytes];
        if (this.num_bytes > 0)
            System.arraycopy(this.data, 0, outobj.data, 0, this.num_bytes); 
        return outobj;
    }
 
}
