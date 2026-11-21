"use client";
import { Alumni } from "@/lib/api";

interface Props {
  alumni: Alumni;
  onClick: () => void;
}

export default function AlumniCard({ alumni, onClick }: Props) {
  const initials = alumni.name
    .split(" ")
    .map((n) => n[0])
    .join("")
    .slice(0, 2)
    .toUpperCase();

  return (
    <button
      onClick={onClick}
      className="alumni-card group text-left w-full"
    >
      <div className="card-inner">
        {/* Avatar */}
        <div className="avatar-wrap">
          {alumni.photo_url ? (
            <img
              src={alumni.photo_url}
              alt={alumni.name}
              className="avatar-img"
              onError={(e) => {
                (e.target as HTMLImageElement).style.display = "none";
                (e.target as HTMLImageElement).nextElementSibling?.removeAttribute("style");
              }}
            />
          ) : null}
          <div
            className="avatar-fallback"
            style={alumni.photo_url ? { display: "none" } : undefined}
          >
            {initials}
          </div>
        </div>

        {/* Info */}
        <div className="card-body">
          <h3 className="alumni-name">{alumni.name}</h3>
          <p className="alumni-role">
            {alumni.current_role || "—"}
            {alumni.current_company ? ` · ${alumni.current_company}` : ""}
          </p>
          <div className="alumni-tags">
            {alumni.department && <span className="tag tag-dept">{alumni.department}</span>}
            {alumni.batch && <span className="tag tag-batch">{alumni.batch}</span>}
            {alumni.city && <span className="tag tag-city">📍 {alumni.city}</span>}
          </div>
        </div>
      </div>
    </button>
  );
}