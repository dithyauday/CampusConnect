"use client";
import { useEffect } from "react";
import { Alumni } from "@/lib/api";

interface Props {
  alumni: Alumni | null;
  onClose: () => void;
}

export default function AlumniModal({ alumni, onClose }: Props) {
  useEffect(() => {
    if (!alumni) return;
    const onKey = (e: KeyboardEvent) => e.key === "Escape" && onClose();
    window.addEventListener("keydown", onKey);
    document.body.style.overflow = "hidden";
    return () => {
      window.removeEventListener("keydown", onKey);
      document.body.style.overflow = "";
    };
  }, [alumni, onClose]);

  if (!alumni) return null;

  const initials = alumni.name.split(" ").map((n) => n[0]).join("").slice(0, 2).toUpperCase();

  const row = (label: string, value: string | number | undefined) =>
    value ? (
      <div className="detail-row">
        <span className="detail-label">{label}</span>
        <span className="detail-value">{value}</span>
      </div>
    ) : null;

  return (
    <div className="modal-backdrop" onClick={onClose}>
      <div className="modal-panel" onClick={(e) => e.stopPropagation()}>
        <button className="modal-close" onClick={onClose}>✕</button>

        {/* Header */}
        <div className="modal-header">
          <div className="modal-avatar">
            {alumni.photo_url ? (
              <img src={alumni.photo_url} alt={alumni.name} className="modal-avatar-img"
                onError={(e) => {
                  (e.target as HTMLImageElement).style.display = "none";
                  (e.target as HTMLImageElement).nextElementSibling?.removeAttribute("style");
                }}
              />
            ) : null}
            <div className="modal-avatar-fallback" style={alumni.photo_url ? { display: "none" } : undefined}>
              {initials}
            </div>
          </div>
          <div>
            <h2 className="modal-name">{alumni.name}</h2>
            <p className="modal-subtitle">
              {[alumni.current_role, alumni.current_company].filter(Boolean).join(" @ ")}
            </p>
          </div>
        </div>

        {/* Details */}
        <div className="detail-grid">
          {row("Department", alumni.department)}
          {row("Course", alumni.course)}
          {row("Batch", alumni.batch)}
          {row("Grad Year", alumni.graduation_year || undefined)}
          {row("From Year", alumni.from_year || undefined)}
          {row("City", alumni.city)}
          {row("Company", alumni.current_company)}
          {row("Role", alumni.current_role)}
        </div>

        {/* LinkedIn */}
        {alumni.linkedin_url && (
          <a
            href={alumni.linkedin_url}
            target="_blank"
            rel="noopener noreferrer"
            className="linkedin-btn"
          >
            View LinkedIn Profile →
          </a>
        )}
      </div>
    </div>
  );
}