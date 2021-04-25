using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.EntityFrameworkCore;

namespace BoilerMonitorServer.DBContext
{
    public class BoilerContext : DbContext
    {
        public BoilerContext(DbContextOptions<BoilerContext> options)
            : base(options)
        { }

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            modelBuilder.Entity<Temperature>()
                .HasKey(o => o.ID);
        }

        public DbSet<Temperature> Temperatures { get; set; }
    }
}
