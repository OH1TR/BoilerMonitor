using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;
using System.ComponentModel.DataAnnotations;

namespace BoilerMonitorServer.Model
{
    public class Temperature
    {
        public int ID { get; set; }
        public DateTime Time { get; set; }
        public byte Point { get; set; }
        public float Value { get; set; }
    }
}
