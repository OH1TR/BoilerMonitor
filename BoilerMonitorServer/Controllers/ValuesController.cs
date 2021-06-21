﻿using BoilerMonitorServer.DBContext;
using DTO;
using Microsoft.AspNetCore.Mvc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace BoilerMonitorServer.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class ValuesController : ControllerBase
    {
        private readonly BoilerContext _context;

        public ValuesController(BoilerContext context)
        {
            _context = context;
        }

        // POST api/values
        [HttpPost]
        public void Post([FromBody] Temperature[] values)
        {
            foreach (var value in values)
            {
                if (_context.Temperatures.Where(i => i.Point == value.Point && i.Time == value.Time).Count() == 0)
                    _context.Temperatures.Add(value);
            }
            _context.SaveChanges();
        }
        [HttpGet("latest")]
        public LatestValues Latest()
        {
            LatestValues retval = new LatestValues() { Time = DateTime.MaxValue };

            var val1 = _context.Temperatures.Where(i => i.Point == 0).OrderByDescending(i => i.Time).Take(1).FirstOrDefault();
            var val2 = _context.Temperatures.Where(i => i.Point == 2).OrderByDescending(i => i.Time).Take(1).FirstOrDefault();
            var smoke = _context.Temperatures.Where(i => i.Point == 3).OrderByDescending(i => i.Time).Take(1).FirstOrDefault();

            if (val1 != null)
            {
                retval.Temp1 = val1.Value;
                retval.Time = retval.Time > val1.Time ? val1.Time : retval.Time;
            }

            if (val2 != null)
            {
                retval.Temp2 = val2.Value;
                retval.Time = retval.Time > val2.Time ? val2.Time : retval.Time;
            }

            if (smoke != null)
            {
                retval.Smoke = smoke.Value;
                retval.Time = retval.Time > smoke.Time ? smoke.Time : retval.Time;
            }

            return (retval);
        }

        [HttpGet]
        public Temperature Get()
        {
            return _context.Temperatures.Where(i => i.Point == 0).OrderByDescending(i => i.Time).Take(1).FirstOrDefault();
        }

        [HttpGet]
        [Route("get12hData")]
        public IEnumerable<Trace> Get12hData()
        {

            DateTime now = DateTime.UtcNow.FloorSeconds();
            DateTime minus12 = DateTime.UtcNow.AddHours(-12);
            DateTime[] times = new DateTime[12 * 60];
            double[] values0 = new double[12 * 60];
            double[] values2 = new double[12 * 60];

            for (int i = 0; i < times.Length; i++)
                times[i] = minus12.AddMinutes(i).ToLocalTime();

            var data = _context.Temperatures.Where(i => i.Time > minus12).ToArray();
            foreach (var m in data.GroupBy(t => new { t.Time }).Select(i => new
            {
                i.Key.Time,
                Temp0 = i.Where(i => i.Point == 0).Average(p => (double?)p.Value) ?? 0,
                Temp1 = i.Where(i => i.Point == 1).Average(p => (double?)p.Value) ?? 0,
                Temp2 = i.Where(i => i.Point == 2).Average(p => (double?)p.Value) ?? 0,
                Temp3 = i.Where(i => i.Point == 3).Average(p => (double?)p.Value) ?? 0
            }))
            {
                int index = (int)((m.Time.FloorSeconds() - minus12).TotalMinutes);
                if (index > 0 && index < times.Length)
                {
                    if (m.Temp0 != 0)
                        values0[index] = m.Temp0;
                    else
                        values0[index] = values0[index - 1];

                    if (m.Temp2 != 0)
                        values2[index] = m.Temp2;
                    else
                        values2[index] = values2[index - 1];
                }
            }

            // Last slot is not always filled.
            if (values0[values0.Length - 1] == 0)
                values0[values0.Length - 1] = values0[values0.Length - 2];

            if (values2[values2.Length - 1] == 0)
                values2[values2.Length - 1] = values2[values2.Length - 2];

            var result= new Trace[] { new Trace() { x = times, y = values0, name = "Trace0" }, new Trace() { x = times, y = values2, name = "Trace2" } };
            return result;
        }
    }
}
