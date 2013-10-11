/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class robot_plan_constraint_checked_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public drc.robot_plan_t robot_plan;
    public int num_constraints;
    public int num_states;
    public byte constraints_satisfied[][];
    public byte CoM_constraint_satisfied;
 
    public robot_plan_constraint_checked_t()
    {
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0xb8becf4a8bea4283L;
 
    public static final byte NOT_OK = (byte) 0;
    public static final byte OK = (byte) 1;

    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.robot_plan_constraint_checked_t.class))
            return 0L;
 
        classes.add(drc.robot_plan_constraint_checked_t.class);
        long hash = LCM_FINGERPRINT_BASE
             + drc.robot_plan_t._hashRecursive(classes)
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
 
        this.robot_plan._encodeRecursive(outs); 
 
        outs.writeInt(this.num_constraints); 
 
        outs.writeInt(this.num_states); 
 
        for (int a = 0; a < this.num_constraints; a++) {
            if (this.num_states > 0)
                outs.write(this.constraints_satisfied[a], 0, num_states);
        }
 
        outs.writeByte(this.CoM_constraint_satisfied); 
 
    }
 
    public robot_plan_constraint_checked_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public robot_plan_constraint_checked_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.robot_plan_constraint_checked_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.robot_plan_constraint_checked_t o = new drc.robot_plan_constraint_checked_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        this.utime = ins.readLong();
 
        this.robot_plan = drc.robot_plan_t._decodeRecursiveFactory(ins);
 
        this.num_constraints = ins.readInt();
 
        this.num_states = ins.readInt();
 
        this.constraints_satisfied = new byte[(int) num_constraints][(int) num_states];
        for (int a = 0; a < this.num_constraints; a++) {
            ins.readFully(this.constraints_satisfied[a], 0, num_states);        }
 
        this.CoM_constraint_satisfied = ins.readByte();
 
    }
 
    public drc.robot_plan_constraint_checked_t copy()
    {
        drc.robot_plan_constraint_checked_t outobj = new drc.robot_plan_constraint_checked_t();
        outobj.utime = this.utime;
 
        outobj.robot_plan = this.robot_plan.copy();
 
        outobj.num_constraints = this.num_constraints;
 
        outobj.num_states = this.num_states;
 
        outobj.constraints_satisfied = new byte[(int) num_constraints][(int) num_states];
        for (int a = 0; a < this.num_constraints; a++) {
            if (this.num_states > 0)
                System.arraycopy(this.constraints_satisfied[a], 0, outobj.constraints_satisfied[a], 0, this.num_states);        }
 
        outobj.CoM_constraint_satisfied = this.CoM_constraint_satisfied;
 
        return outobj;
    }
 
}
