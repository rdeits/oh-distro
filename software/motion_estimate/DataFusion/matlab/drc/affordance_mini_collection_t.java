/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class affordance_mini_collection_t implements lcm.lcm.LCMEncodable
{
    public byte naffs;
    public drc.affordance_mini_t affs[];
 
    public affordance_mini_collection_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x57117357bf4c8ca0L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.affordance_mini_collection_t.class))
            return 0L;
 
        classes.add(drc.affordance_mini_collection_t.class);
        long hash = LCM_FINGERPRINT_BASE
             + drc.affordance_mini_t._hashRecursive(classes)
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
        outs.writeByte(this.naffs); 
 
        for (int a = 0; a < this.naffs; a++) {
            this.affs[a]._encodeRecursive(outs); 
        }
 
    }
 
    public affordance_mini_collection_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public affordance_mini_collection_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.affordance_mini_collection_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.affordance_mini_collection_t o = new drc.affordance_mini_collection_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.naffs = ins.readByte();
 
        this.affs = new drc.affordance_mini_t[(int) naffs];
        for (int a = 0; a < this.naffs; a++) {
            this.affs[a] = drc.affordance_mini_t._decodeRecursiveFactory(ins);
        }
 
    }
 
    public drc.affordance_mini_collection_t copy()
    {
        drc.affordance_mini_collection_t outobj = new drc.affordance_mini_collection_t();
        outobj.naffs = this.naffs;
 
        outobj.affs = new drc.affordance_mini_t[(int) naffs];
        for (int a = 0; a < this.naffs; a++) {
            outobj.affs[a] = this.affs[a].copy();
        }
 
        return outobj;
    }
 
}
