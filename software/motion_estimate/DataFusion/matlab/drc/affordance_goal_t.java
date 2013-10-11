/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class affordance_goal_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public String aff_type;
    public int aff_uid;
    public int num_dofs;
    public String dof_name[];
    public double dof_value[];
 
    public affordance_goal_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0xf4ca9572daac2625L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.affordance_goal_t.class))
            return 0L;
 
        classes.add(drc.affordance_goal_t.class);
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
 
        __strbuf = new char[this.aff_type.length()]; this.aff_type.getChars(0, this.aff_type.length(), __strbuf, 0); outs.writeInt(__strbuf.length+1); for (int _i = 0; _i < __strbuf.length; _i++) outs.write(__strbuf[_i]); outs.writeByte(0); 
 
        outs.writeInt(this.aff_uid); 
 
        outs.writeInt(this.num_dofs); 
 
        for (int a = 0; a < this.num_dofs; a++) {
            __strbuf = new char[this.dof_name[a].length()]; this.dof_name[a].getChars(0, this.dof_name[a].length(), __strbuf, 0); outs.writeInt(__strbuf.length+1); for (int _i = 0; _i < __strbuf.length; _i++) outs.write(__strbuf[_i]); outs.writeByte(0); 
        }
 
        for (int a = 0; a < this.num_dofs; a++) {
            outs.writeDouble(this.dof_value[a]); 
        }
 
    }
 
    public affordance_goal_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public affordance_goal_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.affordance_goal_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.affordance_goal_t o = new drc.affordance_goal_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        char[] __strbuf = null;
        this.utime = ins.readLong();
 
        __strbuf = new char[ins.readInt()-1]; for (int _i = 0; _i < __strbuf.length; _i++) __strbuf[_i] = (char) (ins.readByte()&0xff); ins.readByte(); this.aff_type = new String(__strbuf);
 
        this.aff_uid = ins.readInt();
 
        this.num_dofs = ins.readInt();
 
        this.dof_name = new String[(int) num_dofs];
        for (int a = 0; a < this.num_dofs; a++) {
            __strbuf = new char[ins.readInt()-1]; for (int _i = 0; _i < __strbuf.length; _i++) __strbuf[_i] = (char) (ins.readByte()&0xff); ins.readByte(); this.dof_name[a] = new String(__strbuf);
        }
 
        this.dof_value = new double[(int) num_dofs];
        for (int a = 0; a < this.num_dofs; a++) {
            this.dof_value[a] = ins.readDouble();
        }
 
    }
 
    public drc.affordance_goal_t copy()
    {
        drc.affordance_goal_t outobj = new drc.affordance_goal_t();
        outobj.utime = this.utime;
 
        outobj.aff_type = this.aff_type;
 
        outobj.aff_uid = this.aff_uid;
 
        outobj.num_dofs = this.num_dofs;
 
        outobj.dof_name = new String[(int) num_dofs];
        if (this.num_dofs > 0)
            System.arraycopy(this.dof_name, 0, outobj.dof_name, 0, this.num_dofs); 
        outobj.dof_value = new double[(int) num_dofs];
        if (this.num_dofs > 0)
            System.arraycopy(this.dof_value, 0, outobj.dof_value, 0, this.num_dofs); 
        return outobj;
    }
 
}
