"use client";
import { useState, useEffect, useCallback } from "react";
import { fetchAlumni, Alumni } from "@/lib/api";
import AlumniCard from "@/app/components/AlumniCard";
import AlumniModal from "@/app/components/AlumniModal";

export default function Home() {
  const [alumni, setAlumni]       = useState<Alumni[]>([]);
  const [loading, setLoading]     = useState(true);
  const [selected, setSelected]   = useState<Alumni | null>(null);

  const [search, setSearch]       = useState("");
  const [batch, setBatch]         = useState("");
  const [department, setDepartment] = useState("");
  const [company, setCompany]     = useState("");

  const load = useCallback(async () => {
    setLoading(true);
    try {
      const data = await fetchAlumni({ search, batch, department, company });
      setAlumni(data);
    } catch (e) {
      console.error(e);
    } finally {
      setLoading(false);
    }
  }, [search, batch, department, company]);

  useEffect(() => {
    const t = setTimeout(load, 300);
    return () => clearTimeout(t);
  }, [load]);

  return (
    <main className="main-wrap">
      {/* Header */}
      <header className="site-header">
        <div className="header-inner">
          <div>
            <h1 className="site-title">CampusConnect</h1>
            <p className="site-sub">Alumni Network</p>
          </div>
          <div className="header-count">
            {!loading && <span>{alumni.length} alumni</span>}
          </div>
        </div>
      </header>

      {/* Filters */}
      <section className="filters-bar">
        <input
          className="filter-input filter-search"
          placeholder="Search name or department…"
          value={search}
          onChange={(e) => setSearch(e.target.value)}
        />
        <input
          className="filter-input"
          placeholder="Batch (e.g. 2023)"
          value={batch}
          onChange={(e) => setBatch(e.target.value)}
        />
        <input
          className="filter-input"
          placeholder="Department"
          value={department}
          onChange={(e) => setDepartment(e.target.value)}
        />
        <input
          className="filter-input"
          placeholder="Company"
          value={company}
          onChange={(e) => setCompany(e.target.value)}
        />
      </section>

      {/* Grid */}
      <section className="alumni-grid">
        {loading ? (
          Array.from({ length: 12 }).map((_, i) => (
            <div key={i} className="skeleton-card" />
          ))
        ) : alumni.length === 0 ? (
          <div className="empty-state">No alumni found.</div>
        ) : (
          alumni.map((a) => (
            <AlumniCard key={a.uid} alumni={a} onClick={() => setSelected(a)} />
          ))
        )}
      </section>

      <AlumniModal alumni={selected} onClose={() => setSelected(null)} />
    </main>
  );
}